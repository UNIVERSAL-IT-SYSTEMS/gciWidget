#ifndef _STATICTEXT_H
#define _STATICTEXT_H

#include <gciWidget.h>

class staticText : public gciWidget {
	public:
	    staticText(TFT &dev, Touch &ts, File &file, uint32_t offset, int x, int y) : 
            gciWidget(&dev, &ts, &file, offset, x, y) {}
		staticText(TFT *dev, Touch *ts, File *file, uint32_t offset, int x, int y) :
            gciWidget(dev, ts, file, offset, x, y) {}
		void render(int x, int y);
};

#endif
