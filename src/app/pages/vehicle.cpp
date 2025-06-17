#include <QPalette>
#include <QSerialPortInfo>
#include <cstdio>
#include <qboxlayout.h>
#include <qcolor.h>
#include <qgridlayout.h>
#include <qicon.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qwidget.h>

#include "app/config.hpp"
#include "app/pages/vehicle.hpp"
#include "app/window.hpp"
#include "app/widgets/XMFramedWidget.hpp"
#include "obd/conversions.hpp"
#include "canbus/elm327.hpp"
#include "plugins/vehicle_plugin.hpp"

Gauge::Gauge(units_t units, QFont value_font, QFont unit_font, Gauge::Orientation orientation, int rate,
             std::vector<Command> cmds, int precision, obd_decoder_t decoder, QWidget *parent)
: QWidget(parent)
{
    Config *config = Config::get_instance();
    ICANBus *bus;
    switch(config->get_vehicle_can_bus()){
        //ELM327 USB
        case ICANBus::VehicleBusType::ELM327USB:
            bus = (ICANBus *)elm327::get_usb_instance();
            break;
        //ELM327 Bluetooth
        case ICANBus::VehicleBusType::ELM327BT:
            bus = (ICANBus *)elm327::get_bt_instance();
            break;
        //SocketCAN
        case ICANBus::VehicleBusType::SocketCAN:
        default:
            bus = (ICANBus *)SocketCANBus::get_instance();
            break;
    }

    using namespace std::placeholders;
    std::function<void(QByteArray)> callback = std::bind(&Gauge::can_callback, this, std::placeholders::_1);

    bus->registerFrameHandler(cmds[0].frame.frameId()+0x9, callback);
    DASH_LOG(info)<<"[Gauges] Registered frame handler for id "<<(cmds[0].frame.frameId()+0x9);

    this->si = config->get_si_units();

    this->rate = rate;
    this->precision = precision;

    this->cmds = cmds;
    this->decoder = decoder;

    QBoxLayout *layout;
    if (orientation == BOTTOM)
        layout = new QVBoxLayout(this);
    else
        layout = new QHBoxLayout(this);

    value_label = new QLabel(this->null_value(), this);
    value_label->setFont(value_font);
    value_label->setAlignment(Qt::AlignCenter);

    QLabel *unit_label = new QLabel(this->si ? units.second : units.first, this);
    unit_label->setFont(unit_font);
    unit_label->setAlignment(Qt::AlignCenter);

    this->timer = new QTimer(this);
    connect(this->timer, &QTimer::timeout, [this, bus, cmds]() {
        for (auto cmd : cmds) {
            bus->writeFrame(cmd.frame);
        }
    });

    connect(config, &Config::si_units_changed, [this, units, unit_label](bool si) {
        this->si = si;
        unit_label->setText(this->si ? units.second : units.first);
        value_label->setText(this->null_value());
    });

    layout->addStretch(6);
    layout->addWidget(value_label);
    layout->addStretch(1);
    layout->addWidget(unit_label);
    layout->addStretch(4);
}

void Gauge::can_callback(QByteArray payload){
    Response resp = Response(payload);
    for(auto cmd : cmds){
        if(cmd.frame.payload().at(2) == resp.PID){
            value_label->setText(this->format_value(this->decoder(cmd.decoder(resp), this->si)));
        }
    }
}

QString Gauge::format_value(double value)
{
    if (this->precision == 0)
        return QString::number((int)value);
    else
        return QString::number(value, 'f', this->precision);
}

QString Gauge::null_value()
{
    QString null_str = "-";
    if (this->precision > 0)
        null_str += ".-";
    else
        null_str += '-';

    return null_str;
}

VehiclePage::VehiclePage(Arbiter &arbiter, QWidget *parent)
    : QTabWidget(parent)
    , Page(arbiter, "Vehicle", "directions_car", true, this)
{
    this->tabBar()->hide();
}

void VehiclePage::init()
{
    this->addTab(new DataTab(this->arbiter, this), "Data");
    this->config = Config::get_instance();

    for (auto device : QCanBus::instance()->availableDevices("socketcan"))
        this->can_devices.append(device.name());

    for (auto port : QSerialPortInfo::availablePorts())
        this->serial_devices.append(port.systemLocation());

    connect(&this->arbiter.system().bluetooth, &Bluetooth::init, [this]{
        for (auto device: this->arbiter.system().bluetooth.get_devices())
        {
            if(device->isPaired()){
                this->paired_bt_devices.insert(device->name(), device->address());
            }
        }
    });

    this->get_plugins();
    this->active_plugin = new QPluginLoader(this);
    Dialog *dialog = new Dialog(this->arbiter, true, this->window());
    dialog->set_body(this->dialog_body());
    QPushButton *load_button = new QPushButton("load");
    connect(load_button, &QPushButton::clicked, [this]() { this->load_plugin(); });
    dialog->set_button(load_button);

    // QPushButton *settings_button = new QPushButton(this);
    // settings_button->setFlat(true);
    // this->arbiter.forge().iconize("settings", settings_button, 24);
    // connect(settings_button, &QPushButton::clicked, [dialog]() { dialog->open(); });
    // this->setCornerWidget(settings_button);

    this->load_plugin();
}

QWidget *VehiclePage::dialog_body() 
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QStringList plugins = this->plugins.keys();
    this->plugin_selector = new Selector(plugins, this->config->get_vehicle_plugin(), this->arbiter.forge().font(14), this->arbiter, widget, "unloader");

    layout->addWidget(this->si_units_row_widget(), 1);
    layout->addWidget(Session::Forge::br(), 1);
    layout->addWidget(this->can_bus_toggle_row(), 1);

    QStringList devices;
    switch(config->get_vehicle_can_bus()){
        //ELM327 USB
        case ICANBus::VehicleBusType::ELM327USB:
            devices = this->serial_devices;
            break;
        //ELM327 Bluetooth
        case ICANBus::VehicleBusType::ELM327BT:
            
            break;
        //SocketCAN
        case ICANBus::VehicleBusType::SocketCAN:
        default:
            devices = this->can_devices;
            break;
    }
   
    Selector *interface_selector = new Selector(devices, this->config->get_vehicle_interface(), this->arbiter.forge().font(14), this->arbiter, widget, "disabled");
    interface_selector->setVisible((this->can_devices.size() > 0) || (this->serial_devices.size() > 0) || (this->paired_bt_devices.size() > 0));
    connect(interface_selector, &Selector::item_changed, [this](QString item){
        if(this->config->get_vehicle_can_bus()==ICANBus::VehicleBusType::ELM327BT && item != QString("disabled"))
        {
            this->config->set_vehicle_interface(this->paired_bt_devices[item]);
        }
        else
        {
            this->config->set_vehicle_interface(item);
        }
    });
    connect(this->config, &Config::vehicle_can_bus_changed, [this, interface_selector](int state){
        switch(state){
            //ELM327 USB
            case ICANBus::VehicleBusType::ELM327USB:
                interface_selector->set_options(this->serial_devices);
                break;
            //ELM327 Bluetooth
            case ICANBus::VehicleBusType::ELM327BT:
                interface_selector->set_options(this->paired_bt_devices.keys());
                break;
            //SocketCAN
            case ICANBus::VehicleBusType::SocketCAN:
            default:
                interface_selector->set_options(this->can_devices);
                break;
        }
    });
    connect(&this->arbiter.system().bluetooth, &Bluetooth::init, [this, interface_selector]{
        interface_selector->setVisible((this->can_devices.size() > 0) || (this->serial_devices.size() > 0) || (this->paired_bt_devices.size() > 0));
        if(this->config->get_vehicle_can_bus()==ICANBus::VehicleBusType::ELM327BT){
            QString current = this->config->get_vehicle_interface();
            interface_selector->set_options(this->paired_bt_devices.keys());
            if(current != "disabled")
                interface_selector->set_current(this->paired_bt_devices.key(current));

        }
    });
    layout->addWidget(interface_selector, 1);

    layout->addWidget(Session::Forge::br(), 1);
    layout->addWidget(this->plugin_selector, 1);

    return widget;
}

QWidget *VehiclePage::can_bus_toggle_row()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Interface", widget);
    layout->addWidget(label, 1);

    QGroupBox *group = new QGroupBox();
    QVBoxLayout *group_layout = new QVBoxLayout(group);

    ICANBus::VehicleBusType can_bus_selected = this->config->get_vehicle_can_bus();
    QRadioButton *socketcan_button = new QRadioButton("SocketCAN", group);
    socketcan_button->setChecked(can_bus_selected==ICANBus::VehicleBusType::SocketCAN);
    socketcan_button->setEnabled(this->can_devices.size() > 0);
    connect(socketcan_button, &QRadioButton::clicked, [config = this->config]{
        config->set_vehicle_can_bus(ICANBus::VehicleBusType::SocketCAN);
    });
    group_layout->addWidget(socketcan_button);

    QRadioButton *elm_usb_button = new QRadioButton("ELM327 (USB)", group);
    elm_usb_button->setChecked(can_bus_selected==ICANBus::VehicleBusType::ELM327USB);
    elm_usb_button->setEnabled(this->serial_devices.size() > 0);
    connect(elm_usb_button, &QRadioButton::clicked, [config = this->config]{
        config->set_vehicle_can_bus(ICANBus::VehicleBusType::ELM327USB);
    });
    group_layout->addWidget(elm_usb_button);

    QRadioButton *elm_bt_button = new QRadioButton("ELM327 (Bluetooth)", group);
    elm_bt_button->setChecked(can_bus_selected==ICANBus::VehicleBusType::ELM327BT);
    elm_bt_button->setEnabled(false);
    connect(elm_bt_button, &QRadioButton::clicked, [config = this->config]{
        config->set_vehicle_can_bus(ICANBus::VehicleBusType::ELM327BT);
    });
    connect(&this->arbiter.system().bluetooth, &Bluetooth::init, [this, elm_bt_button]{
            elm_bt_button->setEnabled(this->paired_bt_devices.size() > 0);
    });
    group_layout->addWidget(elm_bt_button);

    layout->addWidget(group, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *VehiclePage::si_units_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("SI Units", widget);
    layout->addWidget(label, 1);

    Switch *toggle = new Switch(widget);
    toggle->scale(this->arbiter.layout().scale);
    toggle->setChecked(this->config->get_si_units());
    connect(toggle, &Switch::stateChanged, [config = this->config](bool state) { config->set_si_units(state); });
    layout->addWidget(toggle, 1, Qt::AlignHCenter);

    return widget;
}

void VehiclePage::get_plugins()
{
    for (const QFileInfo &plugin : Session::plugin_dir("vehicle").entryInfoList(QDir::Files)) {
        if (QLibrary::isLibrary(plugin.absoluteFilePath()))
            this->plugins[Session::fmt_plugin(plugin.baseName())] = plugin;
    }
}

void VehiclePage::load_plugin()
{
    if (this->active_plugin->isLoaded())
        this->active_plugin->unload();

    QString key = this->plugin_selector->get_current();
    if (!key.isNull()) {
        this->active_plugin->setFileName(this->plugins[key].absoluteFilePath());

        if (VehiclePlugin *plugin = qobject_cast<VehiclePlugin *>(this->active_plugin->instance())) {
            plugin->dashize(&this->arbiter);
            switch(config->get_vehicle_can_bus()){
                //ELM327 USB
                case ICANBus::VehicleBusType::ELM327USB:
                    plugin->init((ICANBus *)elm327::get_usb_instance());
                    break;
                //ELM327 Bluetooth
                case ICANBus::VehicleBusType::ELM327BT:
                    plugin->init((ICANBus *)elm327::get_bt_instance());
                    break;
                //SocketCAN
                case ICANBus::VehicleBusType::SocketCAN:
                default:
                    plugin->init((ICANBus *)SocketCANBus::get_instance());
                    break;
            }
            for (QWidget *tab : plugin->widgets())
                this->addTab(tab, tab->objectName());
        }
    }
    this->config->set_vehicle_plugin(key);
}

DataTab::DataTab(Arbiter &arbiter, QWidget *parent)
    : QWidget(parent)
    , arbiter(arbiter)
{
    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10,10,10,10);

    androidAutoWidget = new AndroidAutoWidget(this);
    gridLayout->addWidget(androidAutoWidget, 0, 0, 9, 1);

    climateControlsWidget = new ClimateControlsWidget(this);
    gridLayout->addWidget(climateControlsWidget, 9, 0, 9, 1);
    
    clockWidget = new ClockWidget(this->arbiter, this);
    
    QWidget *Drive_Widget = new FramedWidget(QColor(0, 255, 255), this);
    QVBoxLayout* Drive_Layout = new QVBoxLayout(Drive_Widget); 
    
    Drive_Layout->addWidget(new QLabel("Drive"), 0, Qt::AlignCenter);

    mediaPlayerWidget = new MediaPlayerWidget(this->arbiter, this);
    gridLayout->addWidget(mediaPlayerWidget, 0, 4, 20, 1);

    gridLayout->addWidget(clockWidget, 18, 0, 2, 1);
    gridLayout->addWidget(Drive_Widget, 0, 1, 20, 3);

    // --- Old DataTab layout implementation --- //

    // QHBoxLayout *layout = new QHBoxLayout(this);

    // QWidget *driving_data = this->speedo_tach_widget();
    // layout->addWidget(driving_data);
    // layout->addWidget(Session::Forge::br(true));

    // QWidget *engine_data = this->engine_data_widget();
    // layout->addWidget(engine_data);

    // QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
    // sp_left.setHorizontalStretch(5);
    // driving_data->setSizePolicy(sp_left);
    // QSizePolicy sp_right(QSizePolicy::Preferred, QSizePolicy::Preferred);
    // sp_right.setHorizontalStretch(2);
    // engine_data->setSizePolicy(sp_right);
    // for (auto &gauge : this->gauges)
    //     gauge->start();
}

AndroidAutoWidget::AndroidAutoWidget(QWidget *parent)
    : FramedWidget(QColor(0, 255,255), parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this); 
    
    layout->addWidget(new QLabel("Android Auto"), 0, Qt::AlignCenter);
    layout->setContentsMargins(0,0,0,0);

    directionLabel = new QLabel("Next Right on Koernerstrasse");
    directionLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(directionLabel);

    distanceLabel = new QLabel("DistanceToTurn: 300 m");
    distanceLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(distanceLabel);

    layout->addStretch();
    
    QWidget* timeWidget = new QWidget(this);
    timeWidget->setObjectName("timeWidget");
    timeWidget->setStyleSheet("QWidget#timeWidget { background-color: rgba(150, 255, 255, 0.9); }");
    timeWidget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* timelayout = new QHBoxLayout(timeWidget);
    // timelayout->setContentsMargins(8, 4, 8, 4);
    timelayout->setSpacing(0);

    QLabel* tripDurationLabel = new QLabel("34 min");
    QLabel* tripDistanceLabel = new QLabel("12 km");
    QLabel* etaLabel = new QLabel("14:32");

    QString labelStyle = "color: black; font-weight: bold; font-size: 16px;";
    tripDurationLabel->setStyleSheet(labelStyle);
    tripDistanceLabel->setStyleSheet(labelStyle);
    etaLabel->setStyleSheet(labelStyle);

    tripDurationLabel->setAlignment(Qt::AlignCenter);
    tripDistanceLabel->setAlignment(Qt::AlignCenter);
    etaLabel->setAlignment(Qt::AlignCenter);

    // Add labels with equal stretch
    timelayout->addWidget(tripDurationLabel);
    timelayout->addWidget(tripDistanceLabel);
    timelayout->addWidget(etaLabel);
    layout->addWidget(timeWidget);
}

ClimateControlsWidget::ClimateControlsWidget(QWidget *parent)
    : FramedWidget(QColor(0,255,255), parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Climate Controls"), 0, Qt::AlignCenter);
}

ClockWidget::ClockWidget(Arbiter &arbiter, QWidget *parent)
    : FramedWidget(QColor(0, 255, 255), parent)
    , arbiter(arbiter)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    auto clock = new QLabel();
    clock->setFont(this->arbiter.forge().font(16, true));
    clock->setAlignment(Qt::AlignLeft);
    clock->setAlignment(Qt::AlignVCenter);
    layout->addWidget(clock);

    connect(&this->arbiter.system().clock, &Clock::ticked, [clock](QTime time){
        clock->setText(QLocale().toString(time, QLocale::LongFormat));
    });

    QLabel *logo = new QLabel(this);
    QPixmap pixmap("./assets/icons/citroen.svg");
    if (!pixmap.isNull()) {
        logo->setPixmap(pixmap.scaled(150, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logo->setText("Logo not found");
    }
    logo->setAlignment(Qt::AlignRight);
    logo->setAlignment(Qt::AlignVCenter);
    layout->addWidget(logo);
}

MediaPlayerWidget::MediaPlayerWidget(Arbiter &arbiter, QWidget *parent)
    : FramedWidget(QColor(0,255,255), parent)
    , arbiter(arbiter)
{
    QGridLayout *layout = new QGridLayout(this);

    BluezQt::MediaPlayerPtr media_player = this->arbiter.system().bluetooth.get_media_player().second;
    AAHandler *aa_handler = this->arbiter.android_auto().handler;

    QWidget *albumArtPlaceholder = new QWidget(this);
    albumArtPlaceholder->setStyleSheet("background-color:rgb(90, 158, 163);");
    layout->addWidget(albumArtPlaceholder, 0, 0, 6, 1);
    QLabel *albumArt = new QLabel(this);
    layout->addWidget(albumArt, 0, 0, 6, 1);

    QWidget *trackInfo = new QWidget(this);
    QVBoxLayout *trackInfoLayout = new QVBoxLayout(trackInfo);

    QLabel *artist = new QLabel((media_player != nullptr) ? media_player->track().artist() : QString("Artist"), this);
    trackInfoLayout->addWidget(artist);

    QLabel *album = new QLabel((media_player != nullptr) ? media_player->track().album() : QString("Album"), this);
    trackInfoLayout->addWidget(album);

    QLabel *title = new QLabel((media_player != nullptr) ? media_player->track().title() : QString("Title"), this);
    trackInfoLayout->addWidget(title);
    trackInfoLayout->addStretch();
    
    layout->addWidget(trackInfo, 6, 0, 3, 1);

    connect(&this->arbiter.system().bluetooth, &Bluetooth::media_player_track_changed, [artist, album, title](BluezQt::MediaPlayerTrack track){
        artist->setText(track.artist());
        album->setText(track.album());
        title->setText(track.title());
    });
    
    connect(aa_handler, &AAHandler::aa_media_metadata_update, [artist, album, title, albumArt](const aasdk::proto::messages::MediaInfoChannelMetadataData& metadata){
        title->setText(QString::fromStdString(metadata.track_name()));
        if(metadata.has_artist_name()) artist->setText(QString::fromStdString(metadata.artist_name()));
        if(metadata.has_album_name()) album->setText(QString::fromStdString(metadata.album_name()));
        if(metadata.has_album_art()){
            QImage art;
            art.loadFromData(QByteArray::fromStdString(metadata.album_art()));
            albumArt->setPixmap(QPixmap::fromImage(art));
        }
    });

    // Controls Widget untested

    QWidget *controls = new QWidget(this);
    QHBoxLayout *controlsLayout = new QHBoxLayout(controls);

    QPushButton *previous_button = new QPushButton(controls);
    previous_button->setFlat(true);
    this->arbiter.forge().iconize("skip_previous", previous_button, 56);
    connect(previous_button, &QPushButton::clicked, [this]{
        BluezQt::MediaPlayerPtr media_player = this->arbiter.system().bluetooth.get_media_player().second;
        if (media_player != nullptr)
            media_player->previous()->waitForFinished();
    });
    controlsLayout->addWidget(previous_button);

    QPushButton *play_button = new QPushButton(controls);
    play_button->setFlat(true);
    play_button->setCheckable(true);
    bool status = (media_player != nullptr) ? media_player->status() == BluezQt::MediaPlayer::Status::Playing : false;
    play_button->setChecked(status);
    this->arbiter.forge().iconize("play", "pause", play_button, 56);
    connect(play_button, &QPushButton::clicked, [this, play_button](bool checked = false){
        play_button->setChecked(!checked);

        BluezQt::MediaPlayerPtr media_player = this->arbiter.system().bluetooth.get_media_player().second;
        if (media_player != nullptr) {
            if (checked)
                media_player->play()->waitForFinished();
            else
                media_player->pause()->waitForFinished();
        }
    });
    connect(&this->arbiter.system().bluetooth, &Bluetooth::media_player_status_changed, [play_button](BluezQt::MediaPlayer::Status status){
        play_button->setChecked(status == BluezQt::MediaPlayer::Status::Playing);
    });
    controlsLayout->addWidget(play_button);

    QPushButton *forward_button = new QPushButton(controls);
    forward_button->setFlat(true);
    this->arbiter.forge().iconize("skip_next", forward_button, 56);
    connect(forward_button, &QPushButton::clicked, [this]{
        BluezQt::MediaPlayerPtr media_player = this->arbiter.system().bluetooth.get_media_player().second;
        if (media_player != nullptr)
            media_player->next()->waitForFinished();
    });
    controlsLayout->addWidget(forward_button);

    layout->addWidget(controls, 9, 0, 1, 1);
}   

// middle widget contains speed and tacho informaion
QWidget *DataTab::speedo_tach_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addStretch(3);

    QFont speed_value_font(this->arbiter.forge().font(36, true));

    QFont speed_unit_font(this->arbiter.forge().font(16));
    speed_unit_font.setWeight(QFont::Light);
    speed_unit_font.setItalic(true);

    Gauge *speed = new Gauge({"mph", "km/h"}, speed_value_font, speed_unit_font,
                             Gauge::BOTTOM, 100, {cmds.SPEED}, 0,
                             [](double x, bool si) { return si ? x : kph_to_mph(x); }, widget);
    layout->addWidget(speed);
    this->gauges.push_back(speed);

    layout->addStretch(2);

    QFont tach_value_font(this->arbiter.forge().font(24, true));

    QFont tach_unit_font(this->arbiter.forge().font(12));
    tach_unit_font.setWeight(QFont::Light);
    tach_unit_font.setItalic(true);

    Gauge *rpm = new Gauge({"x1000rpm", "x1000rpm"}, tach_value_font,
                           tach_unit_font, Gauge::BOTTOM, 100, {cmds.RPM}, 1,
                           [](double x, bool _) { return x / 1000.0; }, widget);
    layout->addWidget(rpm);
    this->gauges.push_back(rpm);

    layout->addStretch(1);

    return widget;
}

/*  socketcan/elm327 rewrite right now only has support for one PID per gauge, so we can't calculate milage at this point.
    This is because gauges act more as event handlers now for each PID. 
    Multi-PID gauges could feasibly be reimplemented if there was a helper method that stored received values, and only calls
    the gauge update once all values have been updated since last gauge update.

*/

// QWidget *DataTab::mileage_data_widget()
// {
//     QWidget *widget = new QWidget(this);	
//	   QHBoxLayout *layout = new QHBoxLayout(widget);	
//		
//	   QFont value_font(Theme::font_36);	
//	   value_font.setFamily("Titillium Web");	
//		
//	   QFont unit_font(Theme::font_14);	
//	   unit_font.setWeight(QFont::Light);	
//	   unit_font.setItalic(true);
//
//     Gauge *mileage = new Gauge({"mpg", "km/L"}, value_font, unit_font,
//                                Gauge::BOTTOM, 100, {cmds.SPEED, cmds.MAF}, 1,
//                                [](std::vector<double> x, bool si) {
//                                    return (si ? x[0] : kph_to_mph(x[0])) / (si ? gps_to_lph(x[1]) : gps_to_gph(x[1]));
//                                },
//                                widget);
//     layout->addWidget(mileage);
//     this->gauges.push_back(mileage);

//     return widget;
// }

// Contains Coolant temp and engine load widgets
QWidget *DataTab::engine_data_widget()
{
    QWidget *widget = new QWidget(this);
    // FramedWidget *widget = new FramedWidget(QColor(0, 255, 255), this);
    // QVBoxLayout *layout = new QVBoxLayout(widget);
    auto* layout = new QGridLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // layout->addStretch();
    // layout->addWidget(this->coolant_temp_widget());
    // layout->addStretch();
    // layout->addStretch();
    // layout->addWidget(this->engine_load_widget());
    // layout->addStretch();

    layout->addWidget(this->coolant_temp_widget(), 0, 0);
    layout->addWidget(this->engine_load_widget(), 1, 0);

    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 1);

    widget->setLayout(layout);

    return widget;
}

QWidget *DataTab::coolant_temp_widget()
{
    QWidget *widget = new FramedWidget(QColor(0, 255, 255), this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QFont value_font(this->arbiter.forge().font(16, true));

    QFont unit_font(this->arbiter.forge().font(12));
    unit_font.setWeight(QFont::Light);
    unit_font.setItalic(true);

    Gauge *coolant_temp = new Gauge(
        {"°F", "°C"}, value_font, unit_font, Gauge::RIGHT, 5000,
        {cmds.COOLANT_TEMP}, 1, [](double x, bool si) { return si ? x : c_to_f(x); }, widget);
    layout->addWidget(coolant_temp);
    this->gauges.push_back(coolant_temp);

    QFont label_font(this->arbiter.forge().font(10));
    label_font.setWeight(QFont::Light);

    QLabel *coolant_temp_label = new QLabel("coolant", widget);
    coolant_temp_label->setFont(label_font);
    coolant_temp_label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(coolant_temp_label);

    return widget;
}

QWidget *DataTab::engine_load_widget()
{
    // QWidget *widget = new QWidget(this);
    QWidget *widget = new FramedWidget(QColor(0, 255, 255), this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QFont value_font(this->arbiter.forge().font(16, true));

    QFont unit_font(this->arbiter.forge().font(12));
    unit_font.setWeight(QFont::Light);
    unit_font.setItalic(true);

    Gauge *engine_load =
        new Gauge({"%", "%"}, value_font, unit_font, Gauge::RIGHT,
                  500, {cmds.LOAD}, 1, [](double x, bool _) { return x; }, widget);
    layout->addWidget(engine_load);
    this->gauges.push_back(engine_load);

    QFont label_font(this->arbiter.forge().font(10));
    label_font.setWeight(QFont::Light);

    QLabel *engine_load_label = new QLabel("load", widget);
    engine_load_label->setFont(label_font);
    engine_load_label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(engine_load_label);
    return widget;
}
