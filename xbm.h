#pragma once

inline uint8_t xbm_getpixel(const uint8_t* image, const uint8_t iw, const uint8_t ih, uint8_t x, uint8_t y){
	uint8_t bw = (iw-1)/8+1;
	uint16_t i = bw*y;
	i+=(x/8);
	return (image[i]>>(x%8));
}