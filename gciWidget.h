#ifndef _GCI_WIDGET_H
#define _GCI_WIDGET_H

#include <WProgram.h>

#include <TFT.h>
#include <SD.h>

#define PRESS 1
#define RELEASE 2
#define DRAG 3
#define TAP 4
#define REPEAT 5

extern TFT *_globalTFTDevice;

struct gcihdr {
	uint16_t width;
	uint16_t height;
	uint8_t mode;
	uint8_t fdel;
	uint16_t frames;
} __attribute__((packed));

class gciWidget {
	public:
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
        uint32_t _user;

        int _sx;
        int _sy;
        int _ex;
        int _rx;
        int _ry;
        int _ey;
        uint32_t _st;
        uint32_t _et;
        uint32_t _rt;
        int _rp;
        int _rc;

        void (*_press)(gciWidget *);
        void (*_release)(gciWidget *);
        void (*_drag)(gciWidget *);
        void (*_tap)(gciWidget *);
        void (*_repeat)(gciWidget *);

        void handleTouch();

		gciWidget(TFT &dev, Touch &ts, File &file, uint32_t offset, int x, int y);
		gciWidget(TFT *dev, Touch *ts, File *file, uint32_t offset, int x, int y);
		void render();
		virtual void render(int x, int y);
		uint32_t getFrames();
        void setValue(uint32_t val);
        uint32_t getValue();
        void dumpInfo();

        void invalidate();

        void attachEvent(int event, void (*func)(gciWidget *));
        int getEventDX();
        int getEventDY();
        int getEventX();
        int getEventY();

        int getWidth();
        int getHeight();

        void setUserValue(uint32_t v);
        uint32_t getUserValue();
};

struct widgetList {
    class gciWidget *widget;
    char *name;
    uint32_t pages;
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
        int _selectedPage;
        int _requestedPage;
        uint16_t _background;
        
    public:
        gciWidgetSet(TFT &dev, Touch &ts, const char *basename);
        boolean init();
        void render();
        gciWidget *getWidgetByName(const char *name);
        gciWidget *setPage(const char *name, int page);
        gciWidget *setPage(gciWidget *w, int page);
        void unsetPage(const char *name, int page);
        void unsetPage(gciWidget *w, int page);
        void selectPage(int page);
        void setBackground(uint16_t c);
        void invalidate();
};

#define IMG_BUFSZ 256

#include <staticText.h>

#endif
