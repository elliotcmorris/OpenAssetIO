# Sandboxed builds

For convenience, OpenAssetIO provides two different sandboxed build
environments.

## Docker

A convenient way to perform a from-source build is to use the
[OpenAssetIO-build docker image.](https://github.com/OpenAssetIO/OpenAssetIO/pkgs/container/openassetio-build)
This image contains all the dependencies required for building
OpenAssetIO, and functions as a fully configured build environment.

> **Note**
>
> The `openassetio` build environment is almost identical to
> [VFX reference platform CY2022](https://vfxplatform.com/), with just
> a few [added dependencies](BUILDING.md#library-dependencies).
> As such, the `openassetio-build` container is merely a thin wrapper
> around the [ASWF Docker image](https://github.com/AcademySoftwareFoundation/aswf-docker).

[ASWF Docker image](https://github.com/AcademySoftwareFoundation/aswf-docker).

For example, to build and install OpenAssetIO (by default
to a `dist` directory under the build directory) via a container, from
the repository root run

```shell
docker run -v `pwd`:/src ghcr.io/openassetio/openassetio-build bash -c '
  cd /src && \
  cmake -S . -B build && \
  cmake --build build && \
  cmake --install build'
```

The install tree (`dist`) will contain a complete bundle, including the
core C++ shared library, Python extension module (which dynamically
links to the core C++ library) and Python sources.

The created bundle is therefore suitable for use in both C++ and Python
applications.

The docker image also comes with the [test dependencies](BUILDING.md#test-dependencies)
installed. To build and run tests, from the root of the repository run :

```shell
docker run -v `pwd`:/src ghcr.io/openassetio/openassetio-build bash -c '
  cd /src && \
  cmake -S . -B build -DOPENASSETIO_ENABLE_TESTS=ON && \
  ctest --test-dir build'
```

> **Note**
>
> Running tests via `ctest` will automatically perform the build and
> install steps.

## Vagrant

Included for convenience is a [Vagrant](https://www.vagrantup.com/)
configuration for creating reproducible build environments. This is
configured to create a virtual machine that matches the Linux Github CI
environment as close as is feasible.

In order to build and run the tests within a Vagrant VM, assuming
Vagrant is installed and the current working directory is the root of
the repository, run the following

```shell
cd resources/build
vagrant up
# Wait a while...
vagrant ssh
cmake -S openassetio -B build --install-prefix ~/dist \
  --toolchain ~/.conan/conan_paths.cmake
```

Then we can run the usual steps (see above) to build and install
OpenAssetIO, and run the tests (if enabled).

## Steps for creating openassetio-build docker image

The `openassetio-build` docker container is published to
`ghcr.io/openassetio/openassetio-build`. On occasion, it may become
necessary for contributors to rebuild and republish this image, find
instructions on how to accomplish this below.

### Building the image

OpenAssetIO provides a [makefile](https://github.com/OpenAssetIO/OpenAssetIO/blob/main/resources/build/makefile)
to build the docker container. From `resources/build`, run

``` shell
make docker-image
```

> **Note**
> If you find you need to update the version, this can be found inside
> the makefile itself. The `openassetio-build` container is versioned
> according to the VFX reference platform versioning scheme, eg `2022.1`
> for version 1 of the `CY2022` based image.
>

### Deploying the image

To deploy the image, again, we use `make`

```shell# OpenAssetIO Build Resources

`GITHUB_PAT` refers to a [personal access token](https://github.com/settings/tokens).
It must have the `wite:packages` permission.

> Note: If this is a new image, the first time the image is published
> it will be created `private`. You can change the visibility through
> the GitHub web page for the package.
