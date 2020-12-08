#include "Generator.h"
#include <qendian.h>
#include <qmath.h>


Generator::Generator(const QAudioFormat &format
    , qint64 durationUs
    , int sampleRate, bool right)
{
	if (format.isValid())
		GenerateData(format, durationUs, sampleRate, right);
}

void Generator::start()
{
	open(QIODevice::ReadOnly);
}

void Generator::stop()
{
	pos = 0;
	close();
}

void Generator::GenerateData(const QAudioFormat &format, qint64 durationUs, int sampleRate, bool right)
{
	const int channelBytes = format.sampleSize() / 8;
	const int sampleBytes = format.channelCount() * channelBytes;
	qint64 length = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
	                    * durationUs / 1000000;
	Q_ASSERT(length % sampleBytes == 0);
	Q_UNUSED(sampleBytes) // suppress warning in release builds

	buffer.resize(length);
	unsigned char *ptr = reinterpret_cast<unsigned char *>(buffer.data());
	int sampleIndex = 0;

	while (length)
	{
		for (int i = 0; i < format.channelCount(); ++i)
		{
			qreal x = 0;
			//  Generate sine wave on first (left) or second (right) channel and silence on other.
			if ( (i == 0 && !right) ||
			     (i == 1 && right))
			{
				x = qSin(2 * M_PI * sampleRate * sampleIndex / format.sampleRate()) * 0.1;
				sampleIndex++;
			}
			if (format.sampleSize() == 8)
			{
				if (format.sampleType() == QAudioFormat::UnSignedInt)
				{
					const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
					*reinterpret_cast<quint8 *>(ptr) = value;
				}
				else if (format.sampleType() == QAudioFormat::SignedInt)
				{
					const qint8 value = static_cast<qint8>(x * 127);
					*reinterpret_cast<qint8 *>(ptr) = value;
				}
			}
			else if (format.sampleSize() == 16)
			{
				if (format.sampleType() == QAudioFormat::UnSignedInt)
				{
					quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
					if (format.byteOrder() == QAudioFormat::LittleEndian)
						qToLittleEndian<quint16>(value, ptr);
					else
						qToBigEndian<quint16>(value, ptr);
				}
				else if (format.sampleType() == QAudioFormat::SignedInt)
				{
					qint16 value = static_cast<qint16>(x * 32767);
					if (format.byteOrder() == QAudioFormat::LittleEndian)
						qToLittleEndian<qint16>(value, ptr);
					else
						qToBigEndian<qint16>(value, ptr);
				}
			}

			ptr += channelBytes;
			length -= channelBytes;
		}
	}
}

qint64 Generator::readData(char *data, qint64 len)
{
	qint64 total = 0;
	if (!buffer.isEmpty())
	{
		while (len - total > 0)
		{
			const qint64 chunk = qMin((buffer.size() - pos), len - total);
			memcpy(data + total, buffer.constData() + pos, chunk);
			pos = (pos + chunk) % buffer.size();
			total += chunk;
		}
	}
	return total;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
	Q_UNUSED(data);
	Q_UNUSED(len);

	return 0;
}

qint64 Generator::bytesAvailable() const
{
	return buffer.size() + QIODevice::bytesAvailable();
}
