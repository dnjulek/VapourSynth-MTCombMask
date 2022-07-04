#include "VapourSynth4.h"
#include "VSHelper4.h"

typedef struct {
	VSNode* node;
	int Yth1, Yth2;
} MTCombMaskData;

static void CM_C(const uint8_t* srcp, uint8_t* dstp, ptrdiff_t src_stride, ptrdiff_t dst_stride, int w, int h, uint8_t thresinf, const uint8_t thressup) {
	const uint8_t* s = srcp + src_stride;
	const uint8_t* su = srcp;
	const uint8_t* sd = srcp + static_cast<int64_t>(2) * src_stride;
	uint8_t* d = dstp;
	int smod = src_stride - w;
	int dmod = dst_stride - w;
	int x, y;

	int prod;

	for (x = 0; x < w - 0; x++)
	{
		*d = 0;
		d++;
	}

	d += dmod;

	for (y = 1; y < h - 1; y++)
	{
		for (x = 0; x < w - 0; x++)
		{
			prod = (((*(su)-(*s))) * ((*(sd)-(*s))));

			if (prod < thresinf) *d = 0;
			else if (prod > thressup) *d = 255;
			else *d = (prod >> 8);
			s++;
			su++;
			sd++;
			d++;
		}
		d += dmod;
		s += smod;
		su += smod;
		sd += smod;
	}

	for (x = 0; x < w - 0; x++)
	{
		*d = 0;
		d++;
	}
}

static const VSFrame* VS_CC mtcombmaskGetFrame(int n, int activationReason, void* instanceData, void** frameData, VSFrameContext* frameCtx, VSCore* core, const VSAPI* vsapi) {
	MTCombMaskData* d = (MTCombMaskData*)instanceData;

	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		const VSFrame* src = vsapi->getFrameFilter(n, d->node, frameCtx);
		const VSVideoFormat* fi = vsapi->getVideoFrameFormat(src);
		int height = vsapi->getFrameHeight(src, 0);
		int width = vsapi->getFrameWidth(src, 0);

		VSFrame* dst = vsapi->newVideoFrame(fi, width, height, src, core);

		for (int plane = 0; plane < fi->numPlanes; plane++) {
			const uint8_t* srcp = vsapi->getReadPtr(src, plane);
			ptrdiff_t src_stride = vsapi->getStride(src, plane);
			uint8_t* dstp = vsapi->getWritePtr(dst, plane);
			ptrdiff_t dst_stride = vsapi->getStride(dst, plane);
			int h = vsapi->getFrameHeight(src, plane);
			int w = vsapi->getFrameWidth(src, plane);
			CM_C(srcp, dstp, src_stride, dst_stride, w, h, d->Yth1, d->Yth2);
		}

		vsapi->freeFrame(src);

		return dst;
	}

	return NULL;
}

static void VS_CC mtcombmaskFree(void* instanceData, VSCore* core, const VSAPI* vsapi) {
	MTCombMaskData* d = (MTCombMaskData*)instanceData;
	vsapi->freeNode(d->node);
	free(d);
}

static void VS_CC mtcombmaskCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	MTCombMaskData d;
	MTCombMaskData* data;
	int err;

	d.node = vsapi->mapGetNode(in, "clip", 0, 0);
	const VSVideoInfo* vi = vsapi->getVideoInfo(d.node);

	if (vi->format.sampleType != stInteger || vi->format.bitsPerSample != 8) {
		vsapi->mapSetError(out, "MTCombMask: only 8bit integer input supported");
		vsapi->freeNode(d.node);
		return;
	}

	d.Yth1 = vsapi->mapGetIntSaturated(in, "thY1", 0, &err);
	if (err)
		d.Yth1 = 30;

	d.Yth2 = vsapi->mapGetIntSaturated(in, "thY2", 0, &err);
	if (err)
		d.Yth2 = 30;

	if (d.Yth1 > 255 || d.Yth1 < 0) {
		vsapi->mapSetError(out, "MTCombMask: thY1 value should be in range [0;255]");
		vsapi->freeNode(d.node);
		return;
	}

	if (d.Yth2 > 255 || d.Yth2 < 0) {
		vsapi->mapSetError(out, "MTCombMask: thY2 value should be in range [0;255]");
		vsapi->freeNode(d.node);
		return;
	}

	if (d.Yth1 > d.Yth2) {
		vsapi->mapSetError(out, "MTCombMask: thY1 can't be greater than thY2");
		vsapi->freeNode(d.node);
		return;
	}

	data = (MTCombMaskData*)malloc(sizeof(d));
	*data = d;

	VSFilterDependency deps[] = { {d.node, rpGeneral} };
	vsapi->createVideoFilter(out, "MTCombMask", vi, mtcombmaskGetFrame, mtcombmaskFree, fmParallel, deps, 1, data, core);
}

VS_EXTERNAL_API(void) VapourSynthPluginInit2(VSPlugin* plugin, const VSPLUGINAPI* vspapi) {
	vspapi->configPlugin("com.julek.mtcombmask", "mtcm", "This filter produces a mask showing areas that are combed.", VS_MAKE_VERSION(1, 0), VAPOURSYNTH_API_VERSION, 0, plugin);
	vspapi->registerFunction("MTCombMask", "clip:vnode;thY1:int:opt;thY2:int:opt;", "clip:vnode;", mtcombmaskCreate, NULL, plugin);
}