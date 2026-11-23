#!/bin/bash

config_log_dir=LogDir/configLogs
http_log_dir=LogDir/httpParserLogs
evaluation_log_root=LogDir/evaluationLogs
evaluation_dir=evaluation
config_dir="$evaluation_dir/config"
evaluation_html="$evaluation_dir/html"
evaluation_primary=127.0.0.1:18080
evaluation_secondary=127.0.0.2:18081
mkdir -p "$config_log_dir" "$http_log_dir" "$evaluation_log_root" || {
    printf 'Failed to make the log directory.\n' >&2
    exit 1
}

make > LogDir/make.log 2>&1 & #make redirects its stdout into make.log, now stdout(1) is at make.log, 2>&1 then sends stderr(2) into where stdout(1) points at now 
make_pid=$!
trap 'kill "$make_pid" 2>/dev/null; wait "$make_pid" 2>/dev/null; exit 1' INT TERM

if [ -t 1 ]; then
    dots=.
    while kill -0 "$make_pid" 2>/dev/null; do
        printf '\r\033[K\033[33mCOMPILING%s\033[0m' "$dots"
        sleep 0.15
        if [ "$dots" = "......" ]; then
            dots=.
        else
            dots="$dots."
        fi
    done
   	printf '\r\033[K'
fi

wait "$make_pid"
make_status=$?
trap - INT TERM

if [ "$make_status" -ne 0 ]; then
    printf '\033[31mFAILED TO COMPILE\033[0m\n'
    exit "$make_status"
fi

printf '\033[32m--------------    SUCCESSFULLY COMPILED    --------------\033[0m\n\n'
printf '\033[32m------------------    START TESTING    ------------------\033[0m\n\n'
printf 'TESTING CONFIG FILES\n'

port_is_open() 
{
    # A successful TCP connection means something is listening on this port.
    local address="$1"
    timeout 1s bash -c 'true < "/dev/tcp/$1/$2"' bash "${address%:*}" "${address##*:}" 2>/dev/null
}

wait_for_server_port()
{
    local address="$1"
    local attempt

    for ((attempt = 0; attempt < 30; attempt++)); do
        kill -0 "$server_pid" 2>/dev/null || return 1
        if port_is_open "$address"; then
            return 0
        fi
        sleep 0.1
    done
    return 1
}

stop_test_server()
{
    if [ -n "$server_pid" ]; then
        kill "$server_pid" 2>/dev/null
        wait "$server_pid" 2>/dev/null
        server_pid=
    fi
}

test_config() 
{
    local config_file="$1"
    local log_file="$2"
    local expected_error=
    local listeners=()
    local address config_status

    case "${config_file##*/}" in
        basic.config)               listeners=(127.0.0.1:4242) ;;
        mult_ports.config)          listeners=(127.0.0.1:8080 127.0.0.1:8081 127.0.0.1:8082 127.0.0.1:8083) ;;
        redir.config)               listeners=(127.0.0.1:8080) ;;
        standard.config)            listeners=(127.0.0.1:4242 127.0.0.1:8081) ;;
        evaluation.config)          listeners=("$evaluation_primary" "$evaluation_secondary") ;;
        evaluation_overlap.config)  listeners=("$evaluation_primary") ;;
        incomplete.config)          expected_error="server block is missing 'root'." ;;
        missing_arg.config)         expected_error='Missing argument.' ;;
        nested_location.config)     expected_error='nested location blocks are not allowed.' ;;
        port_already_in_use.config)
            listeners=(127.0.0.1:8080)
            expected_error='Address already in use'
            ;;
        too_many_args.config)       expected_error='Too many arguments.' ;;
        *)
            printf 'No expected result defined for %s.\n' "$config_file" > "$log_file"
            return 1
            ;;
    esac

    # An existing listener could make a startup check pass for the wrong process.
    for address in "${listeners[@]}"; do
        if port_is_open "$address"; then
            printf '%s was already in use before the test.\n' "$address" > "$log_file"
            return 1
        fi
    done

    if [ -n "$expected_error" ]; then
        LC_ALL=C timeout -k 1s 3s ./webserv "$config_file" > "$log_file" 2>&1
        config_status=$?
        if [ "$config_status" -eq 1 ] && grep -Fq -- "$expected_error" "$log_file"; then
            return 0
        fi
        printf '\nExpected exit status 1 and "%s". Got exit status %s.\n' \
            "$expected_error" "$config_status" >> "$log_file"
        return 1
    fi

    LC_ALL=C timeout -k 1s 10s ./webserv "$config_file" > "$log_file" 2>&1 &
    server_pid=$!
    for address in "${listeners[@]}"; do
        if ! wait_for_server_port "$address"; then
            stop_test_server
            printf '\nExpected Webserve to listen on %s.\n' "$address" >> "$log_file"
            return 1
        fi
    done
    stop_test_server
    return 0
}

test_http_request()
{
    local test_name="$1"
    local expected_status="$2"
    local request_line="$3"
    local request_body="$4"
    shift 4
    local log_file="$http_log_dir/$test_name.log"
    local request response client_status version actual_status

    # The remaining arguments are individual headers. An empty line ends the header section.
    printf -v request '%s\r\n' "$request_line" "$@" 'Connection: close' ''
    request+="$request_body"
    printf 'Expected HTTP status %s\n\n--- Request ---\n%s\n--- Response ---\n' \
        "$expected_status" "$request" > "$log_file"

    response=$(LC_ALL=C timeout 2s bash -c '
        exec 3<>"/dev/tcp/127.0.0.1/$1" || exit 1
        printf "%s" "$2" >&3 || exit 1

        # Read the response length so an open connection does not delay the test.
        carriage_return=$(printf "\r")
        content_length=
        while IFS= read -r line <&3; do
            printf "%s\n" "$line"
            line=${line%"$carriage_return"}
            if [ -z "$line" ]; then
                if [ -z "$content_length" ]; then
                    printf "Response is missing Content-Length.\n" >&2
                    exit 1
                fi
                IFS= read -r -N "$content_length" body <&3 || exit 1
                printf "%s" "$body"
                exit 0
            fi
            if [[ "${line,,}" =~ ^content-length:[[:blank:]]*([0-9]+)$ ]]; then
                content_length=${BASH_REMATCH[1]}
            fi
        done
        exit 1
    ' bash "$http_port" "$request" 2>> "$log_file")
    client_status=$?
    printf '%s\n' "$response" >> "$log_file"
    read -r version actual_status _ <<< "$response"

    if [ "$client_status" -ne 0 ]; then
        actual_status="client exit $client_status"
    elif [[ "$version" != HTTP/* ]]; then
        actual_status="no HTTP status"
    fi
    printf '\nResult: %s\n' "$actual_status" >> "$log_file"

    if [ "$actual_status" = "$expected_status" ]; then
        printf '\033[32mPASS\033[0m  %s (HTTP %s)\n' "$test_name" "$actual_status"
        http_passed=$((http_passed + 1))
    else
        printf '\033[31mFAIL\033[0m  %s (expected %s, got %s, see %s)\n' \
            "$test_name" "$expected_status" "$actual_status" "$log_file"
        http_failed=$((http_failed + 1))
    fi
}

test_http_parsers()
{
    local server_log="$http_log_dir/server.log"
    local address="127.0.0.1:$http_port"
    local host_header="Host: $address"
    local content_type='Content-Type: multipart/form-data; boundary=webserv-parser-test'
    local form=$'--webserv-parser-test\r\nContent-Disposition: form-data; name="message"\r\n\r\nhello\r\n--webserv-parser-test--\r\n'
    local incomplete_form="${form%$'--webserv-parser-test--\r\n'}"
    local chunked_form

    if port_is_open "$address"; then
        printf '%s was already in use before the test.\n' "$address" > "$server_log"
    else
        LC_ALL=C timeout -k 1s 45s ./webserv "$config_dir/basic.config" > "$server_log" 2>&1 &
        server_pid=$!
    fi
    if [ -z "$server_pid" ] || ! wait_for_server_port "$address"; then
        stop_test_server
        printf '\033[31mFAIL\033[0m  HTTP parser server startup (see %s)\n' "$server_log"
        http_failed=$((http_failed + 1))
        return
    fi

    test_http_request get-home 200 'GET / HTTP/1.1' '' "$host_header"
    test_http_request query-string 200 'GET /index.html?runtime=1 HTTP/1.1' '' "$host_header"
    test_http_request extra-request-line-token 400 'GET / HTTP/1.1 extra' '' "$host_header"
    test_http_request unsupported-method 501 'PUT / HTTP/1.1' '' "$host_header"
    test_http_request unsupported-version 505 'GET / HTTP/1.0' '' "$host_header"

    test_http_request mixed-case-host 200 'GET / HTTP/1.1' '' "hOsT: 127.0.0.1:$http_port"
    test_http_request missing-host 400 'GET / HTTP/1.1' ''
    test_http_request header-without-colon 400 'GET / HTTP/1.1' '' "$host_header" 'X-Test missing-colon'
    test_http_request invalid-content-length 400 'POST /uploads HTTP/1.1' '' \
        "$host_header" "$content_type" 'Content-Length: invalid'
    test_http_request unsupported-content-type 415 'POST /uploads HTTP/1.1' 'test' \
        "$host_header" 'Content-Type: text/plain' 'Content-Length: 4'

    # A well-formed text field reaches the upload handler, which requires a filename (422).
    # This checks that parsing succeeded without creating an uploaded file.
    test_http_request valid-multipart-form 422 'POST /uploads HTTP/1.1' "$form" \
        "$host_header" "$content_type" "Content-Length: ${#form}"
    test_http_request multipart-missing-end 400 'POST /uploads HTTP/1.1' "$incomplete_form" \
        "$host_header" "$content_type" "Content-Length: ${#incomplete_form}"
    printf -v chunked_form '%x\r\n%s\r\n0\r\n\r\n' "${#form}" "$form"
    test_http_request valid-chunked-form 422 'POST /uploads HTTP/1.1' "$chunked_form" \
        "$host_header" "$content_type" 'Transfer-Encoding: chunked'

    test_http_request get-after-parser-tests 200 'GET / HTTP/1.1' '' "$host_header"
    stop_test_server
}

evaluation_result()
{
    local name="$1" log_file="$2" failure="$3"

    if [ -z "$failure" ]; then
        printf '\033[32mPASS\033[0m  %s\n' "$name"
        evaluation_passed=$((evaluation_passed + 1))
    else
        printf '\n%s\n' "$failure" >> "$log_file"
        printf '\033[31mFAIL\033[0m  %s (%s, see %s)\n' "$name" "$failure" "$log_file"
        evaluation_failed=$((evaluation_failed + 1))
    fi
}

evaluation_skip()
{
    printf '\033[33mSKIP\033[0m  %s (%s)\n' "$1" "$2"
    evaluation_skipped=$((evaluation_skipped + 1))
}

test_evaluation_request()
{
    local name="$1" expected_status="$2" assertion="$3"
    shift 3
    local log_file="$evaluation_log_dir/$name.log"
    local body_file="$evaluation_log_dir/$name.body"
    local status location failure=

    # The remaining arguments go to curl. CGI errors may match any 5xx status.
    printf 'Expected HTTP %s\n' "$expected_status" > "$log_file"
    if [ -n "$assertion" ]; then
        printf 'Expected %s\n' "$assertion" >> "$log_file"
    fi
    if ! status=$(curl -q --noproxy '*' --http1.1 -sv --max-time 4 \
        --output "$body_file" --write-out '%{http_code}' "$@" 2>> "$log_file"); then
        failure='curl could not read a complete response'
    elif [[ "$status" != $expected_status ]]; then
        failure="expected HTTP $expected_status, got $status"
    else
        case "$assertion" in
            file:*)
                cmp -s -- "${assertion#file:}" "$body_file" || failure='response body differs from the expected file'
                ;;
            text:*)
                grep -Fq -- "${assertion#text:}" "$body_file" || failure="response body is missing ${assertion#text:}"
                ;;
            location:*)
                location=$(awk 'tolower($0) ~ /^< location:/ {
                    sub(/^< [^:]*:[[:blank:]]*/, ""); sub(/\r$/, ""); print; exit
                }' "$log_file")
                [ "$location" = "${assertion#location:}" ] || failure="expected Location ${assertion#location:}"
                ;;
        esac
    fi
    printf '\nReceived HTTP %s\n' "${status:-none}" >> "$log_file"
    if [ -f "$body_file" ]; then
        printf '\n--- Response body ---\n' >> "$log_file"
        cat "$body_file" >> "$log_file"
        rm -f -- "$body_file"
    fi
    evaluation_result "$name" "$log_file" "$failure"
}

cleanup_evaluation_uploads()
{
    rm -f -- "${evaluation_upload_files[@]}"
    evaluation_upload_files=()
}

test_evaluation_uploads()
{
    local base="$1"
    local upload_name="webserv-${evaluation_log_dir##*/}.txt"
    local large_name="large-$upload_name"
    local small_file="$evaluation_log_dir/upload-input.data"
    local large_file="$evaluation_log_dir/over-limit-input.data"
    local upload_dir="$evaluation_html/uploads"
    local error_page="$evaluation_html/site/not-found.html"
    local file

    for file in "$upload_dir/$upload_name" "$upload_dir/$large_name"; do
        if [ -e "$file" ] || [ -L "$file" ]; then
            evaluation_result upload-setup "$evaluation_log_dir/upload-setup.log" "test filename already exists: $file"
            return
        fi
    done
    evaluation_upload_files=("$upload_dir/$upload_name" "$upload_dir/$large_name")
    printf 'Webserve evaluation upload\n' > "$small_file"
    printf '%02048d\n' 0 > "$large_file"

    test_evaluation_request upload-under-limit 201 '' \
        --form "file=@$small_file;filename=$upload_name" "$base/uploads/"
    test_evaluation_request retrieve-upload 200 "file:$small_file" "$base/uploads/$upload_name"
    test_evaluation_request delete-forbidden 405 '' -X DELETE "$base/read-only/$upload_name"
    test_evaluation_request upload-after-forbidden-delete 200 "file:$small_file" "$base/uploads/$upload_name"
    test_evaluation_request delete-allowed 204 '' -X DELETE "$base/uploads/$upload_name"
    test_evaluation_request deleted-upload 404 "file:$error_page" "$base/uploads/$upload_name"
    test_evaluation_request upload-over-limit 413 '' \
        --form "file=@$large_file;filename=$large_name" "$base/uploads/"
    test_evaluation_request rejected-upload 404 "file:$error_page" "$base/uploads/$large_name"
    cleanup_evaluation_uploads
}

test_evaluation_cgi()
{
    local base="$1"
    local query_url="$base/cgi-bin/echo.py?message=hello"
    local content_type='Content-Type: multipart/form-data; boundary=webserv-eval'
    local input="$evaluation_log_dir/cgi-input.data"
    local expected_get="$evaluation_log_dir/cgi-get.expected"
    local expected_post="$evaluation_log_dir/cgi-post.expected"

    printf '%s\r\n' '--webserv-eval' 'Content-Disposition: form-data; name="message"' \
        '' 'hello from Webserve' '--webserv-eval--' > "$input"
    printf 'method=GET\nquery=message=hello\n' > "$expected_get"
    {
        printf 'method=POST\nquery=message=hello\n'
        cat "$input"
    } > "$expected_post"

    test_evaluation_request cgi-get 200 "file:$expected_get" "$query_url"
    test_evaluation_request cgi-post 200 "file:$expected_post" \
        -H "$content_type" --data-binary "@$input" "$query_url"
    test_evaluation_request cgi-chunked-post 200 "file:$expected_post" \
        -H "$content_type" -H 'Transfer-Encoding: chunked' --data-binary "@$input" "$query_url"
    test_evaluation_request cgi-relative-path 200 "file:$evaluation_dir/cgi/message.data" "$base/cgi-bin/relative.py"
    test_evaluation_request cgi-failure '5??' '' "$base/cgi-bin/fail.py"
    test_evaluation_request site-after-cgi-failure 200 "file:$evaluation_html/site/index.html" "$base/"
}

test_evaluation_stress()
{
    local base="$1"
    local siege_config="$evaluation_log_dir/siege.conf"
    local process_log="$evaluation_log_dir/server-memory.log"
    local log_file run availability failure

    if ! command -v siege >/dev/null; then
        evaluation_skip 'Siege runs' 'siege is unavailable'
        return
    fi
    cat > "$siege_config" <<'SIEGE_CONFIG'
protocol = HTTP/1.1
connection = close
timeout = 3
parser = false
verbose = false
logging = false
show-logfile = false
SIEGE_CONFIG

    printf 'Before Siege (RSS in KiB)\n' > "$process_log"
    ps -o pid,rss,comm --ppid "$server_pid" >> "$process_log" 2>&1
    for run in 1 2; do
        log_file="$evaluation_log_dir/siege-$run.log"
        failure=
        if LC_ALL=C timeout -k 1s 25s siege -R "$siege_config" -b -c 10 -r 100 -j \
            --no-parser "$base/empty.html" > "$log_file" 2>&1; then
            availability=$(LC_ALL=C awk -F'[:,]' '/"availability"/ { print $2 + 0; exit }' "$log_file")
            if ! LC_ALL=C awk -v percent="$availability" 'BEGIN { exit !(percent + 0 > 99.5) }'; then
                failure="expected availability above 99.5%, reported ${availability:-none}"
            fi
        else
            failure='Siege failed or exceeded 25 seconds'
        fi
        evaluation_result "siege-run-$run" "$log_file" "$failure"
        printf '\nAfter Siege run %s (RSS in KiB)\n' "$run" >> "$process_log"
        ps -o pid,rss,comm --ppid "$server_pid" >> "$process_log" 2>&1
    done
    test_evaluation_request site-after-siege 200 "file:$evaluation_html/site/index.html" "$base/"

    if ! command -v ss >/dev/null; then
        evaluation_skip 'connections after Siege' 'ss is unavailable'
        return
    fi
    sleep 0.2
    log_file="$evaluation_log_dir/connections-after-siege.log"
    failure=
    if ! ss -Htan "( sport = :${evaluation_primary##*:} or sport = :${evaluation_secondary##*:} )" > "$log_file" 2>&1; then
        failure='could not inspect TCP connections'
    elif grep -Eq '^(ESTAB|CLOSE-WAIT)[[:space:]]' "$log_file"; then
        failure='client connections are still open'
    fi
    evaluation_result connections-after-siege "$log_file" "$failure"
}

test_evaluation_requirements()
{
    local primary="http://$evaluation_primary"
    local secondary="http://$evaluation_secondary"
    local index="$evaluation_html/site/index.html"
    local other_index="$evaluation_html/other/second.html"
    local server_log="$evaluation_log_dir/server.log"
    local conflict_log="$evaluation_log_dir/overlapping-port.log"
    local address conflict_status

    if ! command -v curl >/dev/null; then
        evaluation_result evaluation-startup "$server_log" 'curl is unavailable'
        return
    fi
    for address in "$evaluation_primary" "$evaluation_secondary"; do
        if port_is_open "$address"; then
            evaluation_result evaluation-startup "$server_log" "$address is already in use"
            return
        fi
    done
    LC_ALL=C timeout -k 1s 180s ./webserv "$config_dir/evaluation.config" > "$server_log" 2>&1 &
    server_pid=$!
    if ! wait_for_server_port "$evaluation_primary"; then
        stop_test_server
        evaluation_result evaluation-startup "$server_log" "Webserve did not listen on $evaluation_primary"
        return
    fi

    test_evaluation_request primary-site 200 "file:$index" "$primary/"
    test_evaluation_request static-stylesheet 200 "file:$evaluation_html/site/style.css" "$primary/style.css"
    test_evaluation_request secondary-site 200 "file:$other_index" "$secondary/"
    test_evaluation_request route-directory-index 200 "file:$other_index" "$primary/other/"
    test_evaluation_request custom-404 404 "file:$evaluation_html/site/not-found.html" "$primary/missing"
    test_evaluation_request default-404 404 'text:404' "$secondary/missing"
    test_evaluation_request listing-enabled 200 'text:second.html' "$primary/browse/"
    test_evaluation_request listing-disabled 403 '' "$primary/private/"
    test_evaluation_request redirect 301 'location:/index.html' "$primary/old-page"
    test_evaluation_request unknown-method 501 '' -X UNKNOWN "$primary/"

    LC_ALL=C timeout -k 1s 3s ./webserv "$config_dir/evaluation_overlap.config" > "$conflict_log" 2>&1
    conflict_status=$?
    if [ "$conflict_status" -eq 1 ] && grep -Fq 'Address already in use' "$conflict_log"; then
        evaluation_result overlapping-port "$conflict_log" ''
    else
        evaluation_result overlapping-port "$conflict_log" "expected exit status 1 and address-in-use error, got $conflict_status"
    fi
    test_evaluation_request site-after-overlap 200 "file:$index" "$primary/"

    test_evaluation_uploads "$primary"
    test_evaluation_cgi "$primary"
    test_evaluation_request empty-page 200 "file:$evaluation_html/site/empty.html" "$primary/empty.html"
    test_evaluation_stress "$primary"
    stop_test_server
}

shopt -s nullglob
config_files=("$config_dir"/*.config)
if [ "${#config_files[@]}" -eq 0 ]; then
    printf 'No config files found in %s/.\n' "$config_dir" >&2
    exit 1
fi

passed=0
failed=0
server_pid=
evaluation_upload_files=()
trap 'stop_test_server; cleanup_evaluation_uploads; exit 1' INT TERM

for config_file in "${config_files[@]}"; do
    config_name="${config_file##*/}"
    log_file="$config_log_dir/${config_name%.config}.log"
    if test_config "$config_file" "$log_file"; then
        printf '\033[32mPASS\033[0m  %s\n' "$config_name"
        passed=$((passed + 1))
    else
        printf '\033[31mFAIL\033[0m  %s (see %s)\n' "$config_name" "$log_file"
        failed=$((failed + 1))
    fi
done

printf '\n%s passed, %s failed. Logs in %s/\n' "$passed" "$failed" "$config_log_dir"

printf '\nTESTING HTTP PARSERS\n'
http_port=4242
http_passed=0
http_failed=0
test_http_parsers
printf '\n%s passed, %s failed. Logs in %s/\n' "$http_passed" "$http_failed" "$http_log_dir"

printf '\nTESTING EVALUATION REQUIREMENTS\n'
evaluation_log_dir=$(mktemp -d "$evaluation_log_root/run.XXXXXX") || exit 1
evaluation_passed=0
evaluation_failed=0
evaluation_skipped=0
test_evaluation_requirements
printf '\n%s passed, %s failed, %s skipped. Logs in %s/\n' \
    "$evaluation_passed" "$evaluation_failed" "$evaluation_skipped" "$evaluation_log_dir"

trap - INT TERM
if [ "$failed" -ne 0 ] || [ "$http_failed" -ne 0 ] || [ "$evaluation_failed" -ne 0 ]; then
    exit 1
fi
exit 0
