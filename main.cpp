#include <QCoreApplication>
#include <QCommandLineParser>
#include <QAudioDeviceInfo>

#include <iostream>
#include <cmath>
#include <unistd.h>

#include "Measurer.h"

using namespace std;

void ListDevices()
{
	{
		auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput);
		cout << "Output devices:\n";
		for (int i = 0; i < devices.count(); i++)
		{
			cout << devices[i].deviceName().toStdString() << "\n";
		}
	}
	{
		auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioInput);
		cout << "Input devices:\n";
		for (int i = 0; i < devices.count(); i++)
		{
			cout << devices[i].deviceName().toStdString() << "\n";
		}
	}
}

QAudioDeviceInfo GetInputDevice(QString name)
{
	auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioInput);
	for (int i = 0; i < devices.count(); i++)
	{
		if (devices[i].deviceName() == name) return devices[i];
	}
	throw (string)("Unable to find device " + name.toStdString());
}

QAudioDeviceInfo GetOutputDevice(QString name)
{
	auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput);
	for (int i = 0; i < devices.count(); i++)
	{
		if (devices[i].deviceName() == name) return devices[i];
	}
	throw (string)("Unable to find device " + name.toStdString());
}

void Measure(QString input_device_name, QString output_device_name, float duration, bool debug)
{
	auto input = GetInputDevice(input_device_name);
	auto output = GetOutputDevice(output_device_name);
	Measurer left(input, output, MeasureChannel::Left, debug);
	float left_result = left.Measure(duration);
	//usleep(duration * 1000000);
	Measurer right(input, output, MeasureChannel::Right, debug);
	float right_result = right.Measure(duration);
	if (debug)
	{
		cerr << "Measure complete, left_result = " << left_result << " right_result = " << right_result << "\n";
	}
	float k = 2 * right_result;
	float v0v = left_result / k;
	// assume r2 = 10K
	float r1 = 10000 / v0v - 10000;
	float t = 1.0 / (log(r1 / 10000) / 4300 + 1 / 298.0) - 273.0;
	if (debug)
	{
	  cerr << "v0v = " << v0v << " r1 = " << r1 << " t = " << t << "\n";
	}
	cout.setf(ios::fixed | ios::showpoint);
	cout.precision(4);
	cout << t << "\n";
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName("soundthermo-cpp");
	QCoreApplication::setApplicationVersion("0.1");

	QCommandLineParser parser;
	parser.setApplicationDescription("Simple measurement of temperature via sound card");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("command", "Command to execute");


	parser.parse(QCoreApplication::arguments());

	const QStringList args = parser.positionalArguments();
	const QString command = args.isEmpty() ? QString() : args.first();
	if (command == "list")
	{
		parser.clearPositionalArguments();
		ListDevices();
	}
	else if (command == "measure")
	{
		QCommandLineOption input_option(QStringList() << "i" << "input", "Name of device to receive input from", "input");
		QCommandLineOption output_option(QStringList() << "o" << "output", "Name of device to send output to", "output");
		QCommandLineOption duration_option(QStringList() << "d" << "duration", "How many seconds to record (uses twice this time)", "duration");
		QCommandLineOption debug_option("debug", "Turn on additional logging");
		parser.addOption(input_option);
		parser.addOption(output_option);
		parser.addOption(duration_option);
		parser.addOption(debug_option);
		parser.process(a);
		if (!parser.isSet(input_option))
		{
			cerr << "You should specify -i (--input)\n";
			return 1;
		}
		if (!parser.isSet(output_option))
		{
			cerr << "You should specify -o (--output)\n";
			return 1;
		}
		QString input_device_name = parser.value(input_option);
		QString output_device_name = parser.value(output_option);
		float duration = 1;
		if (parser.isSet(duration_option)) 
		{
			duration = parser.value(duration_option).toFloat();
		}
		bool debug = false;
		if (parser.isSet(debug_option)) debug = true;
		Measure(input_device_name, output_device_name, duration, debug);
		return 0;
	}
	else
	{
		parser.process(a);
		parser.showHelp(0);
	}
	return 0;
}
