#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QAudioFormat>
#include <QIODevice>

class Generator : public QIODevice
{
	Q_OBJECT

public:
	Generator(const QAudioFormat &format, qint64 durationUs, int sampleRate, bool right = false);

	void start();
	void stop();

	qint64 readData(char *data, qint64 maxlen) override;
	qint64 writeData(const char *data, qint64 len) override;
	qint64 bytesAvailable() const override;

private:
	void GenerateData(const QAudioFormat &format, qint64 durationUs, int sampleRate, bool right);

private:
	qint64 pos = 0;
	QByteArray buffer;
};

#endif // GENERATOR_H
