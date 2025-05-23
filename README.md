# surfio

Surfio is a library for reading and writing surface files. Currently only supports the irap format.

## Installation

```bash
pip install .
```

## Usage

Irap surfaces can be imported using the `import_ascii` and `import_ascii_file` methods of `IrapSurface`.

```python
import surfio


surface = surfio.IrapSurface.import_ascii_file("./file.irap")
print(surface.header.ncol, surface.header.nrow) # 10, 11
print(surface.values.shape) # (10, 11)
```

`import_ascii_file` is equivalent to

```python
with open("./file.irap") as f:
    surface = surfio.IrapSurface.import_ascii(f.read())
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
    surface.export_ascii_file("./file.txt")
```

which is equivalent to:

```python
with open("./file.irap", mode="w") as f:
    f.write(surface.export_ascii())
```

## Development

```bash
pip install -e ".[dev]"
```

Style is enforced via pre-commit:

```bash
pre-commit install
```

## C++ development

To configure the project

```bash
cmake --preset release-posix
```

To build it

```bash
cmake --build --preset release-posix
```

To test it

```bash
ctest --preset release-posix
```
