#include <OpenEXR/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfPixelType.h>
#include <OpenEXR/ImfInputFile.h>

#include <mex.h>


void
parse_args(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs != 1)
		mexErrMsgTxt("Incorrect number of input arguments.");
	if (nlhs != 1)
		mexErrMsgTxt("Incorrect number of output arguments.");
	if (mxIsChar(prhs[0]) != 1)
		mexErrMsgTxt("Invalid Argument. Must be a file path.");
}


template <typename T>
T lin2srgb(T x)
{
	if (x < 0.0) return 0.0;
	if (x > 1.0) return 1.0;

	if (x < 0.0031308)
		return x * 12.92;
	return 1.055 * powf(x, 1.0 / 2.4) - 0.055;
}


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	using namespace Imf;
	using namespace Imath;

	parse_args(nlhs, plhs, nrhs, prhs);
	char *fname = mxArrayToString(prhs[0]);
	try {
		Array2D<float> r, g, b;
		FrameBuffer fb;
		InputFile exr(fname);

		Box2i dw = exr.header().dataWindow();
		int w = dw.max.x - dw.min.x + 1;
		int h = dw.max.y - dw.min.y + 1;
		r.resizeErase(h, w);
		g.resizeErase(h, w);
		b.resizeErase(h, w);

		fb.insert("R", Slice(HALF,
			(char*)(&r[0][0] - dw.min.x - dw.min.y * w),
			sizeof(r[0][0]), sizeof(r[0][0]) * w, 1, 1, 0));

		fb.insert("G", Slice(HALF,
			(char*)(&g[0][0] - dw.min.x - dw.min.y * w),
			sizeof(g[0][0]), sizeof(g[0][0]) * w, 1, 1, 0));

		fb.insert("B", Slice(HALF,
			(char*)(&b[0][0] - dw.min.x - dw.min.y * w),
			sizeof(b[0][0]), sizeof(b[0][0]) * w, 1, 1, 0));

		exr.setFrameBuffer(fb);
		exr.readPixels(dw.min.y, dw.max.y);

		int dims[2];
		dims[0] = h;
		dims[1] = w;
		plhs[0] = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
		plhs[1] = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
		plhs[2] = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);

		double *r_m = mxGetPr(plhs[0]);
		double *g_m = mxGetPr(plhs[1]);
		double *b_m = mxGetPr(plhs[2]);

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int k = j * h + i;
				r_m[k] = lin2srgb<float>(r[i][j]);
				g_m[k] = lin2srgb<float>(g[i][j]);
				b_m[k] = lin2srgb<float>(b[i][j]);
			}
		}
	}
	catch (const std::exception &e) {
		mxFree(fname);
		mexErrMsgTxt(e.what());
	}
}
