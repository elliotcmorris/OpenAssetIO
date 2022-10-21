@echo off
:: Install additional build tools.
pip3 install -r "%WORKSPACE%\resources\build\requirements.txt"
:: Use explicit predictable conan root path, where packages are 
:: installed.
set CONAN_USER_HOME=%USERPROFILE%\conan
:: Create default conan profile so we can configure it before install.
:: Use --force so that if it already exists we don't error out.
conan profile new default --detect --force
conan profile show default
:: Install openassetio third-party dependencies from public Conan Center
:: package repo.
conan install --install-folder "%WORKSPACE%\.conan" ^
              --build=missing "%WORKSPACE%\resources\build"
