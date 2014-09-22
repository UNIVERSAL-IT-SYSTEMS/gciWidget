#include <staticText.h>

static inline uint16_t swaple(uint16_t be) {
	return ((be & 0xFF00) >> 8) | ((be & 0x00FF) << 8);
}

void staticText::render(int x, int y) {
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
	_file->seek(_offset + sizeof(struct gcihdr) - 2);

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
