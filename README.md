# surfio

Surfio is a library for reading and writing surface files. Currently only supports the irap format.

## Installation

```bash
pip install .
```

## Usage

Irap surfaces can be imported using the `from_ascii_file`, `from_ascii_string`, `from_binary_file` and `from_binary_buffer` methods of `IrapSurface`.

```python
import surfio


surface = surfio.IrapSurface.from_ascii_file("./file.irap")
print(surface.header.ncol, surface.header.nrow)  # 10, 11
print(surface.values.shape)  # (10, 11)
```

`from_ascii_file` is equivalent to

```python
with open("./file.irap") as f:
    surface = surfio.IrapSurface.from_ascii_string(f.read())
```

but is more performant.

Exporting irap surfaces can be done with

```python
    surface = surfio.IrapSurface(
        surfio.IrapHeader(
            ncol=3,
            nrow=2,
            xori=0.0,
            yori=0.0,
            xinc=2.0,
            yinc=2.0,
            xmax=2.0,
            ymax=2.0,
            rot=0.0,
            xrot=0.0,
            yrot=0.0,
        ),
        values=np.zeros((3, 2)),
    )
    surface.to_ascii_file("./file.txt")
```

which is equivalent to:

```python
with open("./file.irap", mode="w") as f:
    f.write(surface.to_ascii_file())
```

## Development

We use [uv](https://docs.astral.sh/uv/) to manage the development environment
and its dependencies. See [installing uv](https://docs.astral.sh/uv/getting-started/installation/).
We recommend installing uv using your system's package manager, or into a
small dedicated virtual environment.

Once uv is installed, you can get a development environment by running:

```bash
git clone https://github.com/equinor/surfio
cd surfio
uv sync --all-groups
```

This builds the C++ extension and installs surfio, together with its test and
development dependencies, into a `.venv`.

We use [just](https://github.com/casey/just) as a command runner for common
development tasks. The most useful recipes are:

```bash
uv run just build-python   # (Re)build and install the surfio python package
uv run just test           # Run both the C++ and python test suites
uv run just test-python    # Run only the python test suite
uv run just lint           # Run all linters/formatters via pre-commit
```

Style is enforced via pre-commit. To have it run automatically on each
commit:

```bash
uv run just install-pre-commit-hooks
```

## C++ development

The C++ project can also be configured, built and tested independently of
building/installing the Python package, using `just`:

```bash
uv run just configure-cpp
uv run just build-cpp
uv run just test-cpp
```

which is equivalent to running the underlying `cmake`/`ctest` commands
directly, e.g.

```bash
cmake --preset release-posix
cmake --build --preset release-posix
ctest --preset test-posix
```
