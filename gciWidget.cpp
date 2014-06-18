#include <gciWidget.h>

static inline uint16_t swaple(uint16_t be) {
	return ((be & 0xFF00) >> 8) | ((be & 0x00FF) << 8);
}

void gciWidget::dumpInfo() {
    _dev->print("Offset: ");
    _dev->println(_offset, HEX);
    _dev->print("Dimensions: ");
    _dev->print(_w);
    _dev->print("x");
    _dev->println(_h);
    _dev->print("Location: ");
    _dev->print(_x);
    _dev->print(",");
    _dev->println(_y);
    _dev->print("Frames: ");
    _dev->println(_frames);
    _dev->print("Value: ");
    _dev->println(_value);
}

gciWidget::gciWidget(TFT &dev, Touch &ts, File &file, uint32_t offset, int x, int y) {
	_dev = &dev;
    _ts = &ts;
	_file = &file;
	_offset = offset;
	_file->seek(_offset);
	struct gcihdr hdr;
	_file->read(&hdr, sizeof(struct gcihdr));
	_w = swaple(hdr.width);
	_h = swaple(hdr.height);
	_frames = swaple(hdr.frames);
	_x = x;
	_y = y;
	_framesize = _w * _h;
    _paint = true;
    _visible = true;
    _enabled = true;
    _active = false;

    _press = NULL;
    _release = NULL;
    _drag = NULL;
    _tap = NULL;
}

gciWidget::gciWidget(TFT *dev, Touch *ts, File *file, uint32_t offset, int x, int y) {
	_dev = dev;
    _ts = ts;
	_file = file;
	_offset = offset;
	_file->seek(_offset);
	struct gcihdr hdr;
	_file->read(&hdr, sizeof(struct gcihdr));
	_w = swaple(hdr.width);
	_h = swaple(hdr.height);
	_frames = swaple(hdr.frames);
	_x = x;
	_y = y;
	_framesize = _w * _h;
    _paint = true;
    _visible = true;
    _enabled = true;
    _active = false;

    _press = NULL;
    _release = NULL;
    _drag = NULL;
    _tap = NULL;
}

#define IMG_BUFSZ 256
void gciWidget::render() {
	render(_x, _y);
}

int gciWidget::getEventDX() {
    return _ex - _sx;
}

int gciWidget::getEventDY() {
    return _ey - _sy;
}

int gciWidget::getEventX() {
    return _rx;
}

int gciWidget::getEventY() {
    return _ry;
}

int gciWidget::getWidth() {
    return _w;
}

int gciWidget::getHeight() {
    return _h;
}

void gciWidget::render(int x, int y) {
    if (_visible == false) {
        return;
    }
    handleTouch();
    if (_paint == false) {
        return;
    }

	uint16_t buf[IMG_BUFSZ];

	if (_value > _frames) {
		return;
	}
	_file->seek(_offset + sizeof(struct gcihdr) + (_framesize * _value * 2));

	_dev->openWindow(x, y, _w, _h);
	uint32_t done = 0;
	while (done < _framesize) {
		int chunk = _framesize - done;
		if (chunk > IMG_BUFSZ) chunk = IMG_BUFSZ;				
		_file->read(buf, chunk * 2);
		for (int j = 0; j < chunk; j++) {
			_dev->windowData(swaple(buf[j]));
		}
		done += chunk;
	}
	_dev->closeWindow();
    _paint = false;
}

void gciWidget::handleTouch() {
    boolean inBounds = (
        _ts->x() >= _x && _ts->x() < (_x + _w) &&
        _ts->y() >= _y && _ts->y() < (_y + _h)
    );

    boolean pressed = _ts->isPressed();

    // Press

    if ((pressed && inBounds) && (!_active)) {
        _active = true;
        _st = millis();
        _sx = _ts->x() - _x;
        _sy = _ts->y() - _y;

        if (_press != NULL) {
            _rx = _sx;
            _ry = _sy;
            _press(this);
        }
    }

    // Release
    if ((!pressed && inBounds) && (_active)) {
        _active = false;
        _et = millis();
        _ex = _ts->x() - _x;
        _ey = _ts->y() - _y;
        
        if (_release != NULL) {
            _rx = _ex;
            _ry = _ey;
            _release(this);
        }

        if (((_et - _st) > 10) && ((_et - _st) < 2000)) {
            if (_tap != NULL) {
                _rx = _ex;
                _ry = _ey;
                _tap(this);
            }
        }
    }

    // Release but out of bounds - no tap event
    if ((!pressed && !inBounds) && (_active)) {
        _active = false;
        _et = millis();
        _ex = _ts->x() - _x;
        _ey = _ts->y() - _y;
        
        if (_release != NULL) {
            _rx = _ex;
            _ry = _ey;
            _release(this);
        }
    }

    // Drag
    if ((pressed && inBounds) && _active) {
        _ex = _ts->x() - _x;
        _ey = _ts->y() - _y;
        if (_drag != NULL) {
            _rx = _ex;
            _ry = _ey;
            _drag(this);
        }
        _sx = _ex;
        _sy = _ey;
    }
}

uint32_t gciWidget::getFrames() {
    return _frames;
}

uint32_t gciWidget::getValue() {
    return _value;
}

void gciWidget::setValue(uint32_t val) {
    if (_value == val) {
        return;
    }
    _value = val;
    if (_value >= _frames) {
        _value = _frames - 1;
    }
    _paint = true;
}

void gciWidget::attachEvent(int e, void (*func)(gciWidget *)) {
    switch (e) {
        case PRESS:
            _press = func;
            break;
        case RELEASE:
            _release = func;
            break;
        case DRAG:
            _drag = func;
            break;
        case TAP:
            _tap = func;
            break;
    }
}

// ** Widget set manager ** //

gciWidgetSet::gciWidgetSet(TFT &dev, Touch &ts, const char *basename) {
    _dev = &dev;
    _ts = &ts;
    _basename = basename;
    _widgets = NULL;
}

boolean gciWidgetSet::init() {
    char fn[strlen(_basename) + 5];
    sprintf(fn, "%s.dat", _basename);
    _datfile = SD.open(fn);
    if (!_datfile) return false;
    sprintf(fn, "%s.gci", _basename);
    _gcifile = SD.open(fn);
    if (!_gcifile) return false;

    char tmp[256];
    int pos = 0;

    char *name = NULL;
    int hi = NULL;
    int lo = NULL;
    int x = NULL;
    int y = NULL;

    while (_datfile.available()) {
        char c = _datfile.read();
        if (c == '\r') continue;
        if (c == '\n') {
            // parse

            name = &tmp[1];
            char *qpos = strchr(name, '"');
            *qpos = 0;
            qpos++;
            qpos++; 

            sscanf(qpos, "%x %x %x %x", &lo, &hi, &x, &y);
            
            uint32_t s = (hi << 16) | lo;

            struct widgetList *wl = (struct widgetList *)malloc(sizeof(struct widgetList));
            wl->name = strdup(name);
            wl->next = NULL;
            wl->widget = new gciWidget(_dev, _ts, &_gcifile, s, x, y);

            if (_widgets == NULL) {
                _widgets = wl;
            } else {
                struct widgetList *scan;
                for (scan = _widgets; scan->next; scan = scan->next);
                scan->next = wl;
            }
            
            pos = 0;
            continue;
        }
        tmp[pos++] = c;
        tmp[pos] = 0;
    }
    _datfile.close();
    return true;
}

void gciWidgetSet::render() {
    struct widgetList *scan;
    for (scan = _widgets; scan; scan = scan->next)
        scan->widget->render();
}

gciWidget *gciWidgetSet::getWidgetByName(const char *name) {
    struct widgetList *scan;
    for (scan = _widgets; scan; scan = scan->next) {
        if (!strcmp(scan->name, name)) {
            return scan->widget;
        }
    }
    return NULL;
}