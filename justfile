set unstable

os_type := if os() == "windows" { "windows" } else { "posix" }
container_engine := if which("docker") =~ ".*docker" {"docker"} else {"podman"}

[doc("Configure cmake project")]
configure-cpp:
  uv run cmake --preset release-{{os_type}}

[doc("Build cmake project")]
build-cpp:
  uv run cmake --build --preset release-{{os_type}}

[doc("(Re)install surfio python package")]
build-python:
  uv sync --reinstall-package surfio --all-extras --all-groups

[doc("Test cpp and python")]
test:
  @just test-cpp test-python

[doc("Test cpp lib")]
test-cpp:
  uv run ctest --preset test-{{os_type}}

[doc("Test python package")]
test-python:
  uv run python -m pytest tests/python_module

[doc("Run cibuildwheel")]
test-cibuildwheel $CIBW_CONTAINER_ENGINE=container_engine:
  cibuildwheel --only cp311-manylinux_x86_64

[doc("Compare surfio with xtgeo")]
compare:
  uv run python tests/manual_performance_test/compare.py

[doc("Install pre-commit hooks")]
install-pre-commit-hooks:
  uv run --only-group=dev pre-commit install

[doc("Run all the linters/formatters")]
lint:
  uv run --only-group=dev pre-commit run --hook-stage pre-commit --all-files
