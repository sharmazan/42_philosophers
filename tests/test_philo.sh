#!/usr/bin/env bash

set -u
set -o pipefail

PHILO_BIN="${1:-./philo/philo}"
PHILO_DIR="$(dirname "$PHILO_BIN")"
LOG_DIR="${LOG_DIR:-./philo_test_logs}"
LOG_FILE="${LOG_FILE:-${LOG_DIR}/test_run_$(date +%Y%m%d_%H%M%S).log}"
TMP_DIR="${TMPDIR:-/tmp}/philo_test_$$"

PASS=0
FAIL=0
SKIP=0

TIMEOUT_BIN=""

mkdir -p "$LOG_DIR"
mkdir -p "$TMP_DIR"
: >"$LOG_FILE"
trap 'rm -rf "$TMP_DIR"' EXIT

green() { printf "\033[32m%s\033[0m\n" "$1"; }
red()   { printf "\033[31m%s\033[0m\n" "$1"; }
yellow(){ printf "\033[33m%s\033[0m\n" "$1"; }

pass() {
    green "✅ PASS: $1"
    PASS=$((PASS + 1))
}

fail() {
    red "❌ FAIL: $1"
    FAIL=$((FAIL + 1))
}

skip() {
    yellow "⚪ SKIP: $1"
    SKIP=$((SKIP + 1))
}

find_timeout() {
    if command -v timeout >/dev/null 2>&1; then
        TIMEOUT_BIN="timeout"
    elif command -v gtimeout >/dev/null 2>&1; then
        TIMEOUT_BIN="gtimeout"
    else
        fail "timeout/gtimeout command not found. Install coreutils."
        return 1
    fi
    return 0
}

is_crash_status() {
    case "$1" in
        134|136|139)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

safe_name() {
    printf "%s" "$1" | tr -c 'A-Za-z0-9_-' '_'
}

RUN_OUT=""
RUN_STATUS=0

append_case_log() {
    local name="$1"
    shift

    {
        printf "\n== CASE: %s ==\n" "$name"
        printf "Command:"
        printf " %q" "$PHILO_BIN"
        while [[ $# -gt 0 ]]; do
            printf " %q" "$1"
            shift
        done
        printf "\nStatus: %s\n" "$RUN_STATUS"
        printf -- "-- output --\n"
        if [[ -s "$RUN_OUT" ]]; then
            cat "$RUN_OUT"
        else
            printf "(no output)\n"
        fi
    } >>"$LOG_FILE"
}

log_check_failure() {
    local label="$1"
    shift

    {
        printf "\n== CHECK FAILED: %s ==\n" "$label"
        "$@"
    } >>"$LOG_FILE" 2>&1
}

run_case() {
    local name="$1"
    local limit="$2"
    shift 2

    local file_name
    file_name="$(safe_name "$name")"
    RUN_OUT="${TMP_DIR}/${file_name}.log"

    "$TIMEOUT_BIN" "$limit" "$PHILO_BIN" "$@" >"$RUN_OUT" 2>&1
    RUN_STATUS=$?
    append_case_log "$name" "$@"
}

compile_project() {
    local out="${TMP_DIR}/make.log"

    if [[ -x "$PHILO_BIN" ]]; then
        pass "binary exists: $PHILO_BIN"
        return 0
    fi

    if [[ ! -f "$PHILO_DIR/Makefile" ]]; then
        fail "binary not found and Makefile not found in $PHILO_DIR"
        return 1
    fi

    make -C "$PHILO_DIR" >"$out" 2>&1
    local st=$?

    {
        printf "\n== MAKE ==\n"
        cat "$out"
        printf "Status: %s\n" "$st"
    } >>"$LOG_FILE"

    if [[ $st -ne 0 ]]; then
        fail "make failed"
        return 1
    fi

    if [[ ! -x "$PHILO_BIN" ]]; then
        fail "make succeeded but binary is not executable: $PHILO_BIN"
        return 1
    fi

    pass "make builds philo"
    return 0
}

check_log_format() {
    local out="$1"
    local n="$2"

    awk -v n="$n" '
        NF == 0 { next }

        !($0 ~ /^[0-9]+ [0-9]+ (has taken a fork|is eating|is sleeping|is thinking|died)$/) {
            printf("Bad log format at line %d: %s\n", NR, $0)
            bad = 1
            next
        }

        {
            id = $2 + 0
            if (id < 1 || id > n) {
                printf("Bad philosopher id at line %d: %s\n", NR, $0)
                bad = 1
            }
        }

        END {
            exit bad ? 1 : 0
        }
    ' "$out"
}

check_no_output_after_death() {
    local out="$1"

    awk '
        NF == 0 { next }

        / died$/ {
            deaths++
            if (deaths > 1) {
                printf("More than one death message at line %d: %s\n", NR, $0)
                bad = 1
            }
            seen_death = 1
            next
        }

        seen_death {
            printf("Output after death at line %d: %s\n", NR, $0)
            bad = 1
        }

        END {
            exit bad ? 1 : 0
        }
    ' "$out"
}

check_death_once() {
    local out="$1"

    local count
    count="$(grep -c ' died$' "$out" || true)"

    if [[ "$count" -ne 1 ]]; then
        printf "Expected exactly 1 death message, got %s\n" "$count"
        return 1
    fi

    return 0
}

check_no_death() {
    local out="$1"

    if grep -q ' died$' "$out"; then
        grep ' died$' "$out"
        return 1
    fi

    return 0
}

check_no_eating() {
    local out="$1"

    if grep -q ' is eating$' "$out"; then
        grep ' is eating$' "$out"
        return 1
    fi

    return 0
}

check_no_runtime_output() {
    local out="$1"

    if grep -Eq '^[0-9]+ [0-9]+ (has taken a fork|is eating|is sleeping|is thinking|died)$' "$out"; then
        grep -En '^[0-9]+ [0-9]+ (has taken a fork|is eating|is sleeping|is thinking|died)$' "$out"
        return 1
    fi

    return 0
}

check_error_message() {
    local name="$1"
    local out="$2"

    if [[ ! -s "$out" ]]; then
        printf "No error message printed for invalid args: %s\n" "$name"
        return 1
    fi

    if ! grep -Eiq 'error|invalid|usage|argument|number|positive|integer' "$out"; then
        printf "Output does not look like an error message for invalid args: %s\n" "$name"
        return 1
    fi

    return 0
}

check_min_eats() {
    local out="$1"
    local n="$2"
    local min_eats="$3"

    awk -v n="$n" -v min_eats="$min_eats" '
        / is eating$/ {
            eats[$2]++
        }

        END {
            bad = 0
            for (i = 1; i <= n; i++) {
                if (eats[i] < min_eats) {
                    printf("Philosopher %d ate %d times, expected at least %d\n",
                        i, eats[i], min_eats)
                    bad = 1
                }
            }
            exit bad ? 1 : 0
        }
    ' "$out"
}

check_two_forks_before_eating() {
    local out="$1"

    awk '
        / has taken a fork$/ {
            forks[$2]++
            next
        }

        / is eating$/ {
            if (forks[$2] < 2) {
                printf("Philosopher %s started eating with only %d fork logs before eating at line %d\n",
                    $2, forks[$2], NR)
                bad = 1
            }
            forks[$2] = 0
            next
        }

        END {
            exit bad ? 1 : 0
        }
    ' "$out"
}

check_single_philo_death_time() {
    local out="$1"
    local time_to_die="$2"
    local tolerance="${DEATH_TOLERANCE_MS:-15}"

    awk -v min="$time_to_die" -v max="$((time_to_die + tolerance))" '
        / died$/ {
            found = 1
            ts = $1 + 0
            if (ts < min || ts > max) {
                printf("Death timestamp %dms is outside expected range [%dms, %dms]\n",
                    ts, min, max)
                bad = 1
            }
        }

        END {
            if (!found) {
                print "No death message found"
                exit 1
            }
            exit bad ? 1 : 0
        }
    ' "$out"
}

warn_if_timestamps_decrease() {
    local out="$1"

    awk '
        NF == 0 { next }

        {
            ts = $1 + 0
            if (NR > 1 && ts < prev) {
                printf("Warning: timestamp decreased at line %d: %d < %d\n",
                    NR, ts, prev)
                warned = 1
            }
            prev = ts
        }

        END {
            exit warned ? 1 : 0
        }
    ' "$out" >/dev/null 2>&1

    if [[ $? -ne 0 ]]; then
        yellow "⚠️  Warning: timestamps decrease somewhere"
    fi
}

common_valid_checks() {
    local name="$1"
    local out="$2"
    local n="$3"

    if ! check_log_format "$out" "$n" >/dev/null 2>&1; then
        log_check_failure "$name: bad log format" check_log_format "$out" "$n"
        fail "$name: bad log format"
        return 1
    fi

    if ! check_no_output_after_death "$out" >/dev/null 2>&1; then
        log_check_failure "$name: output after death or multiple deaths" check_no_output_after_death "$out"
        fail "$name: output after death or multiple deaths"
        return 1
    fi

    if ! check_two_forks_before_eating "$out" >/dev/null 2>&1; then
        log_check_failure "$name: philosopher ate without 2 fork logs" check_two_forks_before_eating "$out"
        fail "$name: philosopher ate without 2 fork logs"
        return 1
    fi

    warn_if_timestamps_decrease "$out"
    return 0
}

expect_invalid() {
    local name="$1"
    shift

    run_case "invalid_${name}" "1s" "$@"

    if [[ $RUN_STATUS -eq 124 ]]; then
        fail "invalid args: $name: program did not exit"
        return 1
    fi

    if is_crash_status "$RUN_STATUS"; then
        fail "invalid args: $name: program crashed with status $RUN_STATUS"
        return 1
    fi

    if [[ $RUN_STATUS -eq 0 ]]; then
        fail "invalid args: $name: expected non-zero exit status"
        return 1
    fi

    if ! check_no_runtime_output "$RUN_OUT" >/dev/null 2>&1; then
        log_check_failure "invalid args: $name: program appears to have started simulation" check_no_runtime_output "$RUN_OUT"
        fail "invalid args: $name: program appears to have started simulation"
        return 1
    fi

    if ! check_error_message "$name" "$RUN_OUT" >/dev/null 2>&1; then
        log_check_failure "invalid args: $name: missing or unclear error message" check_error_message "$name" "$RUN_OUT"
        fail "invalid args: $name: missing or unclear error message"
        return 1
    fi

    pass "invalid args: $name"
    return 0
}

test_single_philosopher() {
    local name="single philosopher dies"
    local n=1
    local time_to_die=200

    run_case "$name" "1s" 1 "$time_to_die" 100 100

    if [[ $RUN_STATUS -eq 124 ]]; then
        fail "$name: timeout"
        return 1
    fi

    if is_crash_status "$RUN_STATUS"; then
        fail "$name: crashed with status $RUN_STATUS"
        return 1
    fi

    if ! common_valid_checks "$name" "$RUN_OUT" "$n"; then
        return 1
    fi

    if ! check_death_once "$RUN_OUT" >/dev/null 2>&1; then
        log_check_failure "$name: expected exactly one death" check_death_once "$RUN_OUT"
        fail "$name: expected exactly one death"
        return 1
    fi

    if ! check_no_eating "$RUN_OUT" >/dev/null 2>&1; then
        log_check_failure "$name: one philosopher must not eat" check_no_eating "$RUN_OUT"
        fail "$name: one philosopher must not eat"
        return 1
    fi

    if ! check_single_philo_death_time "$RUN_OUT" "$time_to_die" >/dev/null 2>&1; then
        log_check_failure "$name: death time is wrong" check_single_philo_death_time "$RUN_OUT" "$time_to_die"
        fail "$name: death time is wrong"
        return 1
    fi

    pass "$name"
    return 0
}

test_no_death_with_must_eat() {
    local name="$1"
    local limit="$2"
    local n="$3"
    local must_eat="$4"
    shift 4

    run_case "$name" "$limit" "$@"

    if [[ $RUN_STATUS -eq 124 ]]; then
        fail "$name: timeout; probably did not stop after must_eat"
        return 1
    fi

    if is_crash_status "$RUN_STATUS"; then
        fail "$name: crashed with status $RUN_STATUS"
        return 1
    fi

    if ! common_valid_checks "$name" "$RUN_OUT" "$n"; then
        return 1
    fi

    if ! check_no_death "$RUN_OUT" >/dev/null 2>&1; then
        log_check_failure "$name: unexpected death" check_no_death "$RUN_OUT"
        fail "$name: unexpected death"
        return 1
    fi

    if ! check_min_eats "$RUN_OUT" "$n" "$must_eat" >/dev/null 2>&1; then
        log_check_failure "$name: not all philosophers ate enough" check_min_eats "$RUN_OUT" "$n" "$must_eat"
        fail "$name: not all philosophers ate enough"
        return 1
    fi

    pass "$name"
    return 0
}

test_expected_death() {
    local name="$1"
    local limit="$2"
    local n="$3"
    shift 3

    run_case "$name" "$limit" "$@"

    if [[ $RUN_STATUS -eq 124 ]]; then
        fail "$name: timeout; program did not stop after death"
        return 1
    fi

    if is_crash_status "$RUN_STATUS"; then
        fail "$name: crashed with status $RUN_STATUS"
        return 1
    fi

    if ! common_valid_checks "$name" "$RUN_OUT" "$n"; then
        return 1
    fi

    if ! check_death_once "$RUN_OUT" >/dev/null 2>&1; then
        log_check_failure "$name: expected exactly one death" check_death_once "$RUN_OUT"
        fail "$name: expected exactly one death"
        return 1
    fi

    pass "$name"
    return 0
}

run_helgrind() {
    if [[ "${RUN_HELGRIND:-0}" != "1" ]]; then
        skip "Helgrind data race check disabled. Run with RUN_HELGRIND=1"
        return 0
    fi

    if ! command -v valgrind >/dev/null 2>&1; then
        skip "valgrind not installed"
        return 0
    fi

    local out="${TMP_DIR}/helgrind.log"

    "$TIMEOUT_BIN" "30s" valgrind \
        --tool=helgrind \
        --error-exitcode=99 \
        "$PHILO_BIN" 5 800 200 200 2 >"$out" 2>&1

    local st=$?

    {
        printf "\n== HELGRIND ==\n"
        cat "$out"
        printf "Status: %s\n" "$st"
    } >>"$LOG_FILE"

    if [[ $st -eq 124 ]]; then
        fail "Helgrind: timeout"
        return 1
    fi

    if [[ $st -ne 0 ]]; then
        fail "Helgrind: possible data race or threading issue"
        return 1
    fi

    pass "Helgrind: no reported threading errors"
    return 0
}

run_memcheck() {
    if [[ "${RUN_MEMCHECK:-0}" != "1" ]]; then
        skip "Memcheck disabled. Run with RUN_MEMCHECK=1"
        return 0
    fi

    if ! command -v valgrind >/dev/null 2>&1; then
        skip "valgrind not installed"
        return 0
    fi

    local out="${TMP_DIR}/memcheck.log"

    "$TIMEOUT_BIN" "30s" valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --errors-for-leak-kinds=all \
        --error-exitcode=98 \
        "$PHILO_BIN" 5 800 200 200 2 >"$out" 2>&1

    local st=$?

    {
        printf "\n== MEMCHECK ==\n"
        cat "$out"
        printf "Status: %s\n" "$st"
    } >>"$LOG_FILE"

    if [[ $st -eq 124 ]]; then
        fail "Memcheck: timeout"
        return 1
    fi

    if [[ $st -ne 0 ]]; then
        fail "Memcheck: leak or memory error detected"
        return 1
    fi

    pass "Memcheck: no leaks/errors reported"
    return 0
}

main() {
    echo "Testing binary: $PHILO_BIN"
    echo "Logs: $LOG_FILE"
    echo

    find_timeout || exit 1
    compile_project || true

    echo
    echo "== Invalid argument tests =="

    expect_invalid "no_args"
    expect_invalid "only_philosophers" 5
    expect_invalid "missing_time_to_eat" 5 800
    expect_invalid "missing_time_to_sleep" 5 800 200
    expect_invalid "too_few_args" 5 800 200
    expect_invalid "too_many_args" 5 800 200 200 3 99
    expect_invalid "non_numeric_philosophers" abc 800 200 200
    expect_invalid "non_numeric_time_to_die" 5 abc 200 200
    expect_invalid "non_numeric_time_to_eat" 5 800 abc 200
    expect_invalid "non_numeric_time_to_sleep" 5 800 200 abc
    expect_invalid "non_numeric_must_eat" 5 800 200 200 abc
    expect_invalid "mixed_numeric_time_to_die" 5 80a 200 200
    expect_invalid "mixed_numeric_suffix" 5 800 200 200 3x
    expect_invalid "mixed_numeric_prefix" 5 x3 200 200
    expect_invalid "decimal_value" 5 800.5 200 200
    expect_invalid "empty_argument" 5 "" 200 200
    expect_invalid "spaces_only_argument" 5 "   " 200 200
    expect_invalid "leading_space_argument" 5 " 800" 200 200
    expect_invalid "trailing_space_argument" 5 "800 " 200 200
    expect_invalid "plus_sign_only" 5 + 200 200
    expect_invalid "minus_sign_only" 5 - 200 200
    expect_invalid "negative_philosophers" -5 800 200 200
    expect_invalid "negative_time_to_die" 5 -800 200 200
    expect_invalid "negative_time_to_eat" 5 800 -200 200
    expect_invalid "negative_time_to_sleep" 5 800 200 -200
    expect_invalid "negative_must_eat" 5 800 200 200 -3
    expect_invalid "zero_philosophers" 0 800 200 200
    expect_invalid "zero_time_to_die" 5 0 200 200
    expect_invalid "zero_time_to_eat" 5 800 0 200
    expect_invalid "zero_time_to_sleep" 5 800 200 0
    expect_invalid "zero_must_eat" 5 800 200 200 0
    expect_invalid "overflow_philosophers" 2147483648 800 200 200
    expect_invalid "overflow_time_to_die" 5 2147483648 200 200
    expect_invalid "overflow_time_to_eat" 5 800 2147483648 200
    expect_invalid "overflow_time_to_sleep" 5 800 200 2147483648
    expect_invalid "overflow_must_eat" 5 800 200 200 2147483648
    expect_invalid "long_overflow" 999999999999999999999999 800 200 200
    expect_invalid "int_max_philosophers_unreasonable" 2147483647 800 200 200

    echo
    echo "== Functional mandatory tests =="

    test_single_philosopher

    test_no_death_with_must_eat \
        "2 philosophers finish must_eat" \
        "5s" \
        2 \
        3 \
        2 800 200 200 3

    test_no_death_with_must_eat \
        "4 philosophers finish must_eat" \
        "7s" \
        4 \
        5 \
        4 800 200 200 5

    test_no_death_with_must_eat \
        "5 philosophers finish must_eat" \
        "7s" \
        5 \
        3 \
        5 800 200 200 3

    test_no_death_with_must_eat \
        "50 philosophers stress finish must_eat" \
        "10s" \
        50 \
        2 \
        50 1000 100 100 2

    test_expected_death \
        "death when cycle is impossible" \
        "3s" \
        4 \
        4 310 200 200

    echo
    echo "== Repeated stress tests =="

    for i in 1 2 3 4 5; do
        test_no_death_with_must_eat \
            "repeat stress run ${i}" \
            "8s" \
            5 \
            7 \
            5 800 200 200 7
    done

    echo
    echo "== Optional dynamic analysis =="

    run_helgrind
    run_memcheck

    echo
    echo "== Summary =="
    echo "Passed:  $PASS"
    echo "Failed:  $FAIL"
    echo "Skipped: $SKIP"
    echo "Logs:    $LOG_FILE"

    if [[ $FAIL -ne 0 ]]; then
        exit 1
    fi

    exit 0
}

main "$@"
