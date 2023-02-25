#ifndef AUDIO_DEVICE_SELECTOR_H
#define AUDIO_DEVICE_SELECTOR_H

#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <string>
#include <alsa/asoundlib.h>

class AudioDeviceSelector : public QWidget {
public:
    AudioDeviceSelector();

    std::string getSelectedDevice() const;

private:
    QComboBox* deviceList;
    QPushButton* refreshButton;
    QPushButton* selectButton;
    std::string selectedDevice;

    void refreshDeviceList();
    void onRefreshClicked();
    void onDeviceSelected(int index);
    void onSelectClicked(int index);
};

#endif
