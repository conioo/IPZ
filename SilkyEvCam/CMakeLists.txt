# Określenie minimalnej wymaganej wersji programu CMake
cmake_minimum_required (VERSION 3.10)

# Włączenie nowoczesnych polityk MSVC (np. wsparcie dla "Hot Reload")
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Definicja nazwy projektu CMake
project ("SilkyEvCam")

# Wskazanie ścieżek do Metavision SDK oraz wbudowanego w SDK pakietu OpenCV
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/Prophesee")
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/Prophesee/third_party/share/opencv")
set(MetavisionSDK_DIR "C:/Program Files/Prophesee/share/cmake/MetavisionsSDK")

# Jawne wyszukanie OpenCV (wymagane do obsługi okna i wizualizacji)
find_package(OpenCV REQUIRED)

# Wyszukiwanie pakietu Metavision SDK wraz z wymaganymi komponentami
find_package(MetavisionSDK COMPONENTS base driver core ui REQUIRED)

# Definicja pliku wykonywalnego oraz dołączenie plików źródłowych
add_executable (SilkyEvCam "SilkyEvCam.cpp" "SilkyEvCam.hpp")

# Powiązanie bibliotek Metavision SDK oraz OpenCV z naszym programem
target_link_libraries(SilkyEvCam PRIVATE 
    MetavisionSDK::base 
    MetavisionSDK::driver 
    MetavisionSDK::core 
    MetavisionSDK::ui 
    ${OpenCV_LIBS}
)

# Wymuszenie standardu języka C++20 dla projektu
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SilkyEvCam PROPERTY CXX_STANDARD 20)
endif()

# Automatyczne kopiowanie głównych bibliotek DLL z SDK do folderu wyjściowego
add_custom_command(TARGET SilkyEvCam POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "C:/Program Files/Prophesee/bin"
    $<TARGET_FILE_DIR:SilkyEvCam>
)

# Automatyczne kopiowanie bibliotek zewnętrznych (OpenCV, Boost itp.) z folderu third_party
add_custom_command(TARGET SilkyEvCam POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "C:/Program Files/Prophesee/third_party/bin"
    $<TARGET_FILE_DIR:SilkyEvCam>
)