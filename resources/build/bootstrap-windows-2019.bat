@echo off
:: Install additional build tools.
pip3 install -r "%WORKSPACE%\resources\build\requirements.txt"
set CONAN_USER_HOME=%WORKSPACE%\.conan
:: Create default conan profile so we can configure it before install.
:: Use --force so that if it already exists we don't error out.
conan profile new default --detect --force
conan profile show default
:: Install openassetio third-party dependencies from public Conan Center
:: package repo.
:: Doesn't build properly on Windows without cpython:shared=True. See: https://github.com/conan-io/conan-center-index/issues/9333
conan install -o cpython:shared=True --install-folder "%CONAN_USER_HOME%" --build=missing "%WORKSPACE%\resources\build"
