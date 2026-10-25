#!/usr/bin/env bash
# Run each example from its build directory, where its shaders and textures
# are, for a number of seconds. An example passes if it is still running by
# then, since errors, including validation errors when built with
# GFX_EXIT_ON_ERROR, end it early with a failure status. The raytracer is
# skipped when the driver has no ray tracing.
#
# Usage: run-examples.sh <build directory> [seconds]
set -u

build_dir=$1
seconds=${2:-10}
status=0

vulkaninfo=$(command -v vulkaninfo || command -v vulkaninfoSDK || true)
ray_tracing=false
if [ -n "$vulkaninfo" ] && "$vulkaninfo" 2>/dev/null | grep -q VK_KHR_ray_tracing_pipeline; then
    ray_tracing=true
fi

for example in rasterizer shadows particles raytracer; do
    if [ "$example" = raytracer ] && [ "$ray_tracing" = false ]; then
        echo "::warning title=$example skipped::The driver has no ray tracing pipelines"
        continue
    fi

    dir="$build_dir/examples/$example"
    exe="./$example"
    if [ ! -x "$dir/$exe" ]; then
        exe="./Debug/$example.exe" # Multi-config generators
    fi

    echo "::group::$example"
    (cd "$dir" && exec "$exe") &
    pid=$!
    sleep "$seconds"

    if kill -0 "$pid" 2>/dev/null; then
        kill "$pid"
        wait "$pid" 2>/dev/null
        echo "::endgroup::"
        echo "$example ran for $seconds seconds"
    else
        wait "$pid"
        code=$?
        echo "::endgroup::"
        echo "::error title=$example failed::$example exited within $seconds seconds with status $code"
        status=1
    fi
done

exit $status
