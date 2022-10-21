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

:: In the case that this script is run from an environment (cibuildwheel)
:: that already has python, don't install over them as they'll conflict.
:: We also don't need to install the test dependencies in that case, as
:: they're unused in the release workflow.
set INSTALL_PYTHON=True
set REQUIRE_TEST_DEPS=True
if "%CIBUILDWHEEL%"=="1" ( 
  set INSTALL_PYTHON=False
  set REQUIRE_TEST_DEPS=False
)

:: Install openassetio third-party dependencies from public Conan Center
:: package repo.
conan install --install-folder "%WORKSPACE%\.conan" ^
              --build=missing "%WORKSPACE%\resources\build" ^
              -o testing=%INSTALL_PYTHON% ^
              -o install_cpython=%REQUIRE_TEST_DEPS%
