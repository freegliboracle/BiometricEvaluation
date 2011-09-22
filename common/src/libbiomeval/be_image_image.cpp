/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cmath>
#include <stdexcept>

#include <be_image_image.h>

BiometricEvaluation::Image::Image::Image(
    const uint8_t *data,
    const uint64_t size,
    const Size dimensions,
    const uint32_t depth,
    const Resolution resolution,
    const CompressionAlgorithm::Kind compressionAlgorithm)
    throw (Error::DataError,
    Error::StrategyError) : 
    _dimensions(dimensions),
    _depth(depth),
    _resolution(resolution),
    _data(size),
    _compressionAlgorithm(compressionAlgorithm)
{
	memcpy(_data, data, size);
}

BiometricEvaluation::Image::Image::Image(
    const uint8_t *data,
    const uint64_t size,
    const CompressionAlgorithm::Kind compressionAlgorithm)
    throw (Error::DataError,
    Error::StrategyError) :
    _dimensions(Size()),
    _depth(0),
    _resolution(Resolution()),
    _data(size),
    _compressionAlgorithm(compressionAlgorithm)
{
	memcpy(_data, data, size);
}

BiometricEvaluation::Image::CompressionAlgorithm::Kind
BiometricEvaluation::Image::Image::getCompressionAlgorithm()
    const
{
	return (_compressionAlgorithm);
}

BiometricEvaluation::Image::Resolution
BiometricEvaluation::Image::Image::getResolution()
    const
{
	return (_resolution);
}

BiometricEvaluation::Image::Size
BiometricEvaluation::Image::Image::getDimensions()
    const
{
	return (_dimensions);
}

uint32_t 
BiometricEvaluation::Image::Image::getDepth()
    const
{
	return (_depth);
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::Image::getRawGrayscaleData(
    uint8_t depth)
    const
    throw (Error::DataError,
    Error::ParameterError)
{
	if (depth != 8 && depth != 1)
		throw Error::ParameterError("Invalid value for bit depth");
		
	/* Images that are 8-bit depth are already grayscale */
	if (getDepth() == 8 && depth == 8)
		return (getRawData());

	Memory::AutoArray<uint8_t> rawColor = getRawData();
	Memory::AutoArray<uint8_t> rawGray(getDimensions().xSize * 
	    getDimensions().ySize);
	
	/* Constants from ITU-R BT.601 */
	static const float redFactor = 0.299;
	static const float greenFactor = 0.587;
	static const float blueFactor = 0.114;
	
	uint8_t twoBytes[2];
	uint16_t rValue, bValue, gValue;
	
	for (uint32_t i = 0, j = 0; i < rawColor.size() && j < rawGray.size(); 
	    j++) {
		switch (getDepth()) {
		case 1:
			/* Bitmap images are upped to 8-bit in getRawData() */
			/* FALLTHROUGH */
		case 8:
			/* No conversion needed */
			rawGray[j] = rawColor[i];
			
			i += 1;
			break;
		case 16:
			/* Extract 16-bit value */
			twoBytes[0] = rawColor[i];
			twoBytes[1] = rawColor[i + 1];
			rValue = ((twoBytes[0] << 8) | (twoBytes[1]));
			
			/* Interpolate color in 8-bit depth colorspace */
			rawGray[j] = (uint8_t)valueInColorspace(rValue,
			    max16BitColor, 8);
			
			i += 2;
			break;
		case 24:
			/* Pull Y' component from Y'CbCr */
			rawGray[j] = (uint8_t)((rawColor[i] * redFactor) +
			    (rawColor[i + 1] * greenFactor) +
			    (rawColor[i + 2] * blueFactor));
			    
			i += 3;
			break;
		case 32:
			/* Pull Y' component from Y'CbCr */
			rawGray[j] = (uint8_t)((rawColor[i] * redFactor) +
			    (rawColor[i + 1] * greenFactor) +
			    (rawColor[i + 2] * blueFactor));
			    
			/* Skip alpha channel */
			i += 4;
			break;
		case 48:
			/* Extract 16-bit values */
			twoBytes[0] = rawColor[i];
			twoBytes[1] = rawColor[i + 1];
			rValue = ((twoBytes[0] << 8) | (twoBytes[1]));
		
			twoBytes[0] = rawColor[i + 2];
			twoBytes[1] = rawColor[i + 3];
			gValue = ((twoBytes[0] << 8) | (twoBytes[1]));
		
			twoBytes[0] = rawColor[i + 4];
			twoBytes[1] = rawColor[i + 5];
			bValue = ((twoBytes[0] << 8) | (twoBytes[1]));
						
			/* Interpolate colors in 8-bit depth colorspace */
			rValue = (uint8_t)valueInColorspace(rValue,
			    max48BitColor, 8);
    			gValue = (uint8_t)valueInColorspace(rValue,
			    max48BitColor, 8);
    			bValue = (uint8_t)valueInColorspace(rValue,
			    max48BitColor, 8);
			
			/* Pull Y' component from Y'CbCr */
			rawGray[j] = (uint8_t)(((uint8_t)rValue * redFactor) +
			    ((uint8_t)gValue * greenFactor) +
			    ((uint8_t)bValue * blueFactor));

			i += 6;
			break;
		}
	}
	
	switch (depth) {
	case 1:
		/* Quantize down to black and white */
		for (uint32_t i = 0; i < rawGray.size(); i++) {
			if (rawGray[i] <= 127)
				rawGray[i] = 0x00;
			else
				rawGray[i] = 0xFF;
		}
		break;
	}

	return (rawGray);
}

BiometricEvaluation::Memory::AutoArray<uint8_t>
BiometricEvaluation::Image::Image::getData()
    const
{
	return (_data);
}

void
BiometricEvaluation::Image::Image::setResolution(
    const Resolution resolution)
{
	_resolution = resolution;
}

void
BiometricEvaluation::Image::Image::setDimensions(
    const Size dimensions)
{
	_dimensions = dimensions;
}
	
void
BiometricEvaluation::Image::Image::setDepth(
    const uint32_t depth)
{
	_depth = depth;
}

BiometricEvaluation::Image::Image::~Image()
{

}

inline uint64_t
BiometricEvaluation::Image::Image::valueInColorspace(
    uint64_t color,
    uint64_t maxColorValue,
    uint8_t depth)
{
	/*
	 * Solve for X in:
	 *
	 *          color             X
	 *      ------------- = -------------
	 *      maxColorValue   2^(depth) - 1
	 */

	return ((((uint64_t)pow(2.0, depth) - 1) * color) / maxColorValue);
}

