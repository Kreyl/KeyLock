/*
 * color.h
 *
 *  Created on: 11.02.2012
 *      Author: kreyl
 */

#ifndef COLOR_H_
#define COLOR_H_

// Colors
struct Color_t {
    uint8_t Red, Green, Blue;
    //bool operator == (const Color_t AColor) { return ((this->Red == AColor.Red) && (this->Green == AColor.Green) && (this->Blue == AColor.Blue)); }
    //bool operator != (const Color_t AColor) { return ((this->Red != AColor.Red) || (this->Green != AColor.Green) || (this->Blue != AColor.Blue)); }
    //bool IsOn(void) const { return (Red || Green || Blue); }
} __attribute__ ((packed));

#define clBlack     {0, 0, 0}
#define clRed       {255, 0, 0}
#define clGreen     {0, 255, 0}
#define clBlue      {0, 0, 255}


#endif /* COLOR_H_ */
