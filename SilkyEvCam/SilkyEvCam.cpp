#include "SilkyEvCam.hpp"

int main(int argc, char* argv[]) {

    // Wymuszenie ścieżki do wtyczek CenturyArks w procesie C++
    _putenv_s("MV_HAL_PLUGIN_PATH", "C:\\Program Files\\CenturyArks\\plugins");
    _putenv_s("MV_HAL_PLUGIN_SEARCH_MODE", "PLUGIN_PATH_ONLY");

    // ==========================================
    // SEKCJA KONFIGURACJI
    // ==========================================
    // Jeśli zmienna jest pusta (R"()"), program otwiera pierwszą dostępną kamerę na żywo.
    //std::string input_path = R"()";
    std::string input_path = R"(C:\Users\posce\Documents\C++\IPZ\Samples\recording_2026-10-14_11-43-19.raw)";

    std::cout << "==========================================" << std::endl;
    std::cout << " Aplikacja podgladu zdarzen SilkyEvCam" << std::endl;
    std::cout << "==========================================" << std::endl;

    try {
        Metavision::Camera cam;

        // Inicjalizacja kamery lub pliku w zależności od konfiguracji
        if (input_path.empty()) {
            std::cout << "Inicjalizacja kamery na zywo..." << std::endl;
            cam = Metavision::Camera::from_first_available();
        }
        else {
            std::cout << "Otwieranie pliku nagrania: " << input_path << "..." << std::endl;
            cam = Metavision::Camera::from_file(input_path);
        }

        // Pobranie rozdzielczości sensora kamery
        const int width = cam.geometry().width();
        const int height = cam.geometry().height();
        std::cout << "Rozdzielczosc sensora: " << width << " x " << height << std::endl;

        // Przygotowanie okna OpenCV
        cv::String window_name = "Metavision events (C++)";
        cv::namedWindow(window_name, cv::WINDOW_NORMAL);

        // Bufor na zdarzenia oraz mutex zapewniający bezpieczeństwo wątków
        std::vector<Metavision::EventCD> current_events;
        std::mutex events_mutex;

        // Rejestracja callbacku odbierającego paczki zdarzeń CD (Change Detection)
        cam.cd().add_callback([&](const Metavision::EventCD* begin, const Metavision::EventCD* end) {
            std::lock_guard<std::mutex> lock(events_mutex);
            current_events.insert(current_events.end(), begin, end);
            });

        // Uruchomienie strumieniowania
        cam.start();
        std::cout << "\nStrumieniowanie aktywne. Nacisnij klawisz 'q' lub ESC w oknie podgladu, aby zamknac." << std::endl;

        // Utworzenie ramki wypełnionej szarym tłem (wartość 127, kolor RGB)
        cv::Mat frame(height, width, CV_8UC3, cv::Scalar(127, 127, 127));

        // Główna pętla wizualizacji
        while (cam.is_running()) {
            // SPRAWDZENIE: Czy użytkownik zamknął okno przyciskiem 'X'?
            if (cv::getWindowProperty(window_name, cv::WND_PROP_VISIBLE) < 1) {
                std::cout << "Okno zostało zamknięte przez użytkownika." << std::endl;
                break;
            }

            // Pobranie i wyczyszczenie zebranych zdarzeń
            std::vector<Metavision::EventCD> events_to_process;
            {
                std::lock_guard<std::mutex> lock(events_mutex);
                events_to_process = std::move(current_events);
                current_events.clear();
            }

            // Resetowanie ramki do szarego tła
            frame.setTo(cv::Scalar(127, 127, 127));

            // Rysowanie zdarzeń na ramce (p=1 -> biały, p=0 -> czarny)
            for (const auto& ev : events_to_process) {
                if (ev.x >= 0 && ev.x < width && ev.y >= 0 && ev.y < height) {
                    if (ev.p) {
                        frame.at<cv::Vec3b>(ev.y, ev.x) = cv::Vec3b(255, 255, 255);
                    }
                    else {
                        frame.at<cv::Vec3b>(ev.y, ev.x) = cv::Vec3b(0, 0, 0);
                    }
                }
            }

            // Wyświetlenie ramki w oknie OpenCV
            cv::imshow(window_name, frame);

            // Sprawdzanie naciśnięcia klawisza 'q' lub ESC
            int key = cv::waitKey(1) & 0xFF;
            if (key == 27 || key == 'q') {
                std::cout << "Zatrzymanie na żądanie użytkownika..." << std::endl;
                break;
            }

            // Krótkie uśpienie dla optymalizacji zużycia procesora
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        // Zatrzymanie kamery i zamknięcie okien
        cam.stop();
        cv::destroyAllWindows();
        std::cout << "Program zakończył działanie pomyślnie." << std::endl;

    }
    catch (const Metavision::CameraException& e) {
        std::cerr << "Blad kamery Metavision: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}