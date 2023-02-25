#include <QApplication>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QStyleFactory>
#include <QListWidget>
#include <QPushButton>
#include <iostream>
#include <string>
#include <alsa/asoundlib.h>

class AudioDeviceSelector : public QWidget {
public:
    AudioDeviceSelector() {
        setWindowTitle("Audio Device Selector");

        QVBoxLayout* layout = new QVBoxLayout(this);

        QLabel* label = new QLabel("Select Audio Device:");
        layout->addWidget(label);

        deviceList = new QComboBox();
        layout->addWidget(deviceList);

        refreshButton = new QPushButton("Refresh");
        layout->addWidget(refreshButton);

        selectButton = new QPushButton("Select Device");
        layout->addWidget(selectButton);

        setLayout(layout);

        // Connect signals and slots
        connect(refreshButton, &QPushButton::clicked, this, &AudioDeviceSelector::onRefreshClicked);
        connect(selectButton, &QPushButton::clicked, this, &AudioDeviceSelector::onSelectClicked);
        connect(deviceList, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AudioDeviceSelector::onDeviceSelected);

        // Load the list of audio devices
        refreshDeviceList();
    }

    std::string getSelectedDevice() const {
        return deviceList->currentText().toStdString();
    }

private:
    QComboBox* deviceList;
    QPushButton* refreshButton;
    QPushButton* selectButton;
    std::string selectedDevice;

    void refreshDeviceList() {
        deviceList->clear();

        void **hints;
        if (snd_device_name_hint(-1, "pcm", &hints) != 0) {
            std::cerr << "Error getting audio device hints" << std::endl;
            return;
        }

        int i = 0;
        for (void **hint = hints; *hint; hint++) {
            char *name = snd_device_name_get_hint(*hint, "NAME");
            deviceList->addItem(name);
            free(name);
        }

        snd_device_name_free_hint(hints);
    }

    void onRefreshClicked() {
        refreshDeviceList();
    }

    void onDeviceSelected(int index) {
        selectedDevice = deviceList->itemText(index).toStdString();
    }

    void onSelectClicked(int index) {
        // std::cout << "Selected audio device: " << selectedDevice << std::endl;
        close();
    }
};

// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);

//     AudioDeviceSelector selector;
//     selector.show();
//     app.exec();
//     std::cout << "Selected audio device: " << selector.getSelectedDevice() << std::endl;


//     return 0;
// }
