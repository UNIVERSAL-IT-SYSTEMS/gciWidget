#ifndef _GCI_WIDGET_H
#define _GCI_WIDGET_H

#include <WProgram.h>

#include <TFT.h>
#include <SD.h>

#define PRESS 1
#define RELEASE 2
#define DRAG 3
#define TAP 4

struct gcihdr {
	uint16_t width;
	uint16_t height;
	uint8_t mode;
	uint8_t fdel;
	uint16_t frames;
} __attribute__((packed));

class gciWidget {
	private:
		TFT *_dev;
        Touch *_ts;
        File *_file;
		uint32_t _offset;
		int _x;
		int _y;
		int _w;
		int _h;
		int _frames;
  		uint32_t _framesize;
        int _value;
        boolean _paint;
        boolean _enabled;
        boolean _visible;
        boolean _active;

        int _sx;
        int _sy;
        int _ex;
        int _ey;
        int _st;
        int _et;
        int _rx;
        int _ry;

        void (*_press)(gciWidget *);
        void (*_release)(gciWidget *);
        void (*_drag)(gciWidget *);
        void (*_tap)(gciWidget *);

        void handleTouch();

	public:
		gciWidget(TFT &dev, Touch &ts, File &file, uint32_t offset, int x, int y);
		gciWidget(TFT *dev, Touch *ts, File *file, uint32_t offset, int x, int y);
		void render();
		void render(int x, int y);
		uint32_t getFrames();
        void setValue(uint32_t val);
        uint32_t getValue();
        void dumpInfo();

        void attachEvent(int event, void (*func)(gciWidget *));
        int getEventDX();
        int getEventDY();
        int getEventX();
        int getEventY();

        int getWidth();
        int getHeight();
};

struct widgetList {
    class gciWidget *widget;
    char *name;
    struct widgetList *next;
};

class gciWidgetSet {
    private:
		File _datfile;
		File _gcifile;
        TFT *_dev;
        Touch *_ts;
        struct widgetList *_widgets;
        const char *_basename;
        
    public:
        gciWidgetSet(TFT &dev, Touch &ts, const char *basename);
        boolean init();
        void render();
        gciWidget *getWidgetByName(const char *name);
};

#define IMG_BUFSZ 256

#endif
