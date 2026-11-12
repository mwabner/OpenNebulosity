# Sony / Nikon / Canon – ARM64-Kameravorschau

Stand: 12. November 2026. Repository `mwabner/OpenNebulosity`, lokaler Branch `feature/macos-arm64`.

## Implementierter Stand

Unter **Help → Sony / Nikon / Canon camera test** gibt es eine native Kamerasuche über Apples ImageCaptureCore und libgphoto2. Der experimentelle Dialog kann eine von libgphoto2 erkannte USB-Kamera verbinden, eine Einzelaufnahme mit den an der Kamera eingestellten Parametern auslösen, die Datei in einen gewählten Ordner laden und sie nach Schließen des Dialogs in OpenNebulosity öffnen. Die Verbindung bleibt zwischen den Vorgängen offen. Bestehende Bilddateien auf der Kamera werden nicht gelöscht; der Code ruft keine Löschfunktion auf.

Der Dialog bietet Abbruch einer laufenden Operation. Er verweigert Bulb- und Time-Aufnahmen, weil eine zeitgesteuerte Langzeitbelichtung hier noch nicht implementiert ist. Die Belichtung und das Aufnahmeformat werden an der Kamera eingestellt; dieser erste Dialog verändert sie nicht. Er ergänzt den bisherigen ARM64-Vorschaubuild; die normale Kameraauswahl mit Simulator und die Serienaufnahme sind noch nicht an den neuen Aufnahmeadapter angeschlossen.

macOS kann eine USB-Kamera für den Bildimport belegen. Bei diesem Verbindungsfehler bietet der Dialog ausdrücklich an, die Verbindung freizugeben. Erst nach Zustimmung beendet er den zugehörigen Bildimportdienst des angemeldeten Benutzers. Laufende Kameraimporte können dadurch unterbrochen werden. Keine Dienste werden dauerhaft deaktiviert.

## Tatsächlich mit der Sony Alpha 6000 geprüft

- Kamera: Sony ILCE-6000; Fernsteuerungsmodus mit USB-Kennung `054c:094e`.
- Vorherige USB-Kennung `054c:07c4` führte nicht zur gPhoto2-Fernsteuerungserkennung.
- Kamerasuche im nativen Dialog: erfolgreich, sowohl Apple- als auch gPhoto2-Zugangsweg sichtbar.
- Einstellungen gelesen: M, 1/320 s, Auto-ISO, JPEG Fine, manueller Fokus.
- Eine JPEG-Testaufnahme mit gphoto2 ausgelöst und heruntergeladen: 6000 × 4000 Pixel.
- Für genau eine weitere Aufnahme auf RAW gestellt, ARW aufgenommen/heruntergeladen und anschließend wieder JPEG Fine eingestellt; Rücklesen bestätigte Fine. Keine ISO- oder Belichtungsänderung durch den Test.
- Native LibRaw-Decodierung der ARW-Datei: erfolgreich; Bildbereich 6024 × 4024, Rohdatenbereich 6048 × 4024.
- Dieselbe ARW-Datei in der neuen App geöffnet und als FITS gespeichert. Vergleich aller 24.240.576 Bayer-Sensorpixel mit LibRaw: **0 Abweichungen**.
- Der neue Testdialog wurde im Hauptprogramm geöffnet; ohne Kamera sind Verbindung und Aufnahme korrekt deaktiviert. Fehlerpfade der neuen Sitzung sind getestet.

**Noch ausstehend:** Die positive Aufnahme mit der neu geschriebenen `GPhotoCaptureSession` und dem neuen Aufnahmeknopf muss abschließend mit angeschlossener Kamera getestet werden. Die oben genannten echten Aufnahmen erfolgten über gphoto2 in einer zusammenhängenden Diagnosesitzung, nicht über diesen neuen Knopf. Abbruch während einer echten Aufnahme, Kabelverlust und parallele Importe sind noch nicht hardwaregetestet.

## Behobener RAW-Datenfehler

Der vorherige ARM64-FreeImage-Build enthielt zusätzlich zur aktuellen Homebrew-LibRaw eine alte, eingebettete LibRaw-Kopie. Symbolkollisionen führten bei Bayer-Dateien zu falschen Pixeln. Der ARM64-Build lässt jetzt FreeImages altes RAW-Plugin und dessen LibRawLite-Quellen weg; OpenNebulositys eigener RAW-Import nutzt ausschließlich die aktuelle LibRaw. Das Archiv wird bei Änderungen neu erstellt, sodass entfernte Objektdateien nicht im Archiv verbleiben. Intel-FreeImage und Intel-Target wurden nicht verändert. Der korrigierte ARW/FITS-Vergleich bestätigt die Datenintegrität. Die frühere Vorschau sollte für RAW-Dateien durch diesen neuen Build ersetzt werden.

## Modellabdeckung

Die installierte native Bibliothek ist libgphoto2 2.5.34, das Diagnoseprogramm gphoto2 2.5.32. Der Aufnahmeadapter verwendet die zur Laufzeit gefundenen Modelle statt einer festen Sony-/Nikon-/Canon-Modellliste. `driver-model-support.tsv` enthält die vom installierten Treiber gemeldeten Fähigkeiten. Die Einträge umfassen aktuelle und ältere Modelle sowie unterschiedliche USB-Modi; sie sind keine Liste ausschließlich aktuell verkaufter Kameras und kein Hardwaretestnachweis für OpenNebulosity.

Für die Alpha 6000 meldet der Control-Treiber Aufnahme, Auslösen und Konfiguration, aber keine Live-View-Aufnahme. Beim MTP-Eintrag werden diese Fernsteuerungsfunktionen nicht gemeldet. Nikon- und Canon-Kameras sind für denselben Adapter vorgesehen, aber in dieser Sitzung nicht hardwaregetestet. Auch eine Treiberliste garantiert nicht jede Funktion mit jeder Firmware.

Apples ImageCaptureCore übernimmt Erkennung und bietet einen PTP-Transport. Vollständige Belichtungs- und RAW-Funktionen werden dadurch nicht automatisch herstellerunabhängig. Der erste Aufnahmeweg verwendet daher libgphoto2. Proprietäre SDKs bleiben eine mögliche Ergänzung für nicht abgedeckte Modelle/Funktionen; sie sind für den hier geprüften Alpha-6000-Weg nicht erforderlich.

Quellen, abgerufen am 12. November 2026:

- [Sony ILCE-6000: USB-Verbindung/PC-Fernbedienung](https://helpguide.sony.net/gbmig/45349336/v1/de/contents/TP0000244079.html)
- [gPhoto: unterstützte Kameras und Fernsteuerungsfähigkeiten](https://gphoto.sourceforge.io/proj/libgphoto2/support.php)
- [Apple ImageCaptureCore / ICCameraDevice](https://developer.apple.com/documentation/imagecapturecore/iccameradevice)
- [Sony Remote SDK: unterstützte Geräte](https://support.d-imaging.sony.co.jp/app/sdk/en/index.html)
- [Nikon: aktuelles Remote-Module-SDK](https://sdk.nikonimaging.com/apply/)
- [Canon: EDSDK über USB und CCAPI über Netzwerk](https://asia.canon/en/campaign/developerresources/cap)

## Bauen und Tests

Zusätzlich zu den bisherigen ARM64-Abhängigkeiten benötigt die Kameravorschau native libgphoto2-Bibliotheken unter `/opt/homebrew`; sie wurden auf diesem Rechner über Homebrew installiert. Der bekannte Development-Build läuft erfolgreich, der Xcode-Skriptschutz bleibt eingeschaltet. Es ist weiterhin ein lokaler Vorschau-Build mit Homebrew-Laufzeitabhängigkeiten und Mindestziel macOS 27.0.

`arm64/tests/gphoto_capture_errors.cpp` prüft Sitzungsfehler ohne angeschlossene Kamera. `arm64/tests/raw_fits_roundtrip.cpp` benötigt eine ARW-Datei und den daraus gespeicherten FITS-Export. Die Testaufnahmen liegen außerhalb des Repositorys im Arbeitsordner und werden nicht in Git aufgenommen.

Die vorgefundenen Änderungen wurden vor den Kameraarbeiten gesichert. Intel-Projektobjekte wurden mit dieser Sicherung verglichen. Der nachvollziehbare Entwicklungsstand wird auf `feature/macos-arm64` gepflegt; Buildartefakte und private Testbilder bleiben lokal.
