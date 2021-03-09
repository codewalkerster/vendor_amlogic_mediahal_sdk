/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */



#include <string>
#include <vector>
#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <sys/time.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <thread>
#include <sys/mman.h>
#include "crc32.h"
#include "AmVideoDecBase.h"

static uint32_t kDefaultQueueCount = 32;
using namespace std::chrono_literals;
#define MAX_INSTANCE_MUN  9
#define InputBufferMaxSize (1024 * 1024)

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

typedef enum {
	VFORMAT_UNKNOWN = -1,
	VFORMAT_MPEG12 = 0,
	VFORMAT_MPEG4,
	VFORMAT_H264,
	VFORMAT_MJPEG,
	VFORMAT_REAL,
	VFORMAT_JPEG,
	VFORMAT_VC1,
	VFORMAT_AVS,
	VFORMAT_SW,
	VFORMAT_H264MVC,
	VFORMAT_H264_4K2K,
	VFORMAT_HEVC,
	VFORMAT_H264_ENC,
	VFORMAT_JPEG_ENC,
	VFORMAT_VP9,
	VFORMAT_AVS2,
	VFORMAT_DVES_AVC,
	VFORMAT_DVES_HEVC,
	VFORMAT_MPEG2TS,
	VFORMAT_UNSUPPORT,
	VFORMAT_MAX
} vformat_t;

const char* vformat_to_mime(uint32_t vformat) {
	switch (vformat) {
		case 2:
			return "video/avc";
		case 11:
			return "video/hevc";
		case 14:
			return "video/x-vnd.on2.vp9";
		case 0:
			return "video/mpeg2";
		case 1:
			return "video/mp4v-es";
		case 3:
			return "video/x-motion-jpeg";
		case 6:
			return "video/vc1";
		case 7:
			return "video/avs";
		case 15:
			return "video/avs2";
		default:
			return "";
	}
}

static int sysfs_cmd_control(const char *path, const char *cmdstr)
{
    int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);

    if (fd >= 0) {
        write(fd, cmdstr, strlen(cmdstr));
        close(fd);
        return 0;
    }

    return -1;
}

class VideoDecPlayerExample {
public:
	VideoDecPlayerExample();
	virtual ~VideoDecPlayerExample();
	int init(uint32_t vFmt, uint32_t width = 1920, uint32_t height = 1080, uint32_t framerate = 24);
	void setQueueCount(int32_t queueCount);
	void play(const char* iname, const char* sname,const char* oname, int num);
	void outputBuffer(const char* oname, int num);
	int inputBuffer(uint8_t* buf, uint32_t size, uint64_t timestamp = 0);
	void dumpData(char* buf, uint32_t width, uint32_t height, int32_t bitstreamId);

	/* Implement callback */
	virtual void onOutputFormatChanged(uint32_t requestedNumOfBuffers,
		int32_t width, uint32_t height);
	virtual void onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
		uint32_t width, uint32_t height);
	virtual void onInputBufferDone(int32_t bitstream_buffer_id);
	virtual void onUpdateDecInfo(const uint8_t* info, uint32_t isize);
	virtual void onFlushDone();
	virtual void onResetDone();
	virtual void onError(int32_t error);
	virtual void onEvent(uint32_t event, void* param, uint32_t paramsize);

	class playerCallback : public AmVideoDecCallback {
		public:
			playerCallback(VideoDecPlayerExample *thiz) : mThis(thiz) {}
			virtual ~playerCallback() = default;
			virtual void onOutputFormatChanged(uint32_t requestedNumOfBuffers,
					int32_t width, uint32_t height) override {
				mThis->onOutputFormatChanged(requestedNumOfBuffers, width, height);
			}
			virtual void onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
					uint32_t width, uint32_t height) override {
				mThis->onOutputBufferDone(pictureBufferId, bitstreamId, width, height);
			}
			virtual void onInputBufferDone(int32_t bitstream_buffer_id) override {
				mThis->onInputBufferDone(bitstream_buffer_id);
			}
			virtual void onUserdataReady(const uint8_t* userdata, uint32_t usize) override {
				UNUSED(userdata);
				UNUSED(usize);
			}
			virtual void onUpdateDecInfo(const uint8_t* info, uint32_t isize) override {
				mThis->onUpdateDecInfo(info, isize);
			}
			virtual void onFlushDone() override {
				mThis->onFlushDone();
			}
			virtual void onResetDone() override {
				mThis->onResetDone();
			}
			virtual void onError(int32_t error) override {
				mThis->onError(error);
			}
			virtual void onEvent(uint32_t event, void* param, uint32_t paramsize) override {
				mThis->onEvent(event, param, paramsize);
			}

		private:
			VideoDecPlayerExample * const mThis;
	};

	playerCallback* mCallback;
	AmVideoDecBase* mAmVideoDec;

	std::mutex mFlushedLock;
	std::condition_variable mFlushedCondition;
	bool mready;

	bool out_flag;

	FILE *miFp;
	FILE *msFp;
	FILE *moFp;
	FILE *mcFp;

struct dispWork {
	int64_t bitstreamId;
	int64_t timestamp;
	int32_t pictureBufferId;
	uint32_t width;
	uint32_t height;
};

/*Ion output for non-bufferqueue*/
struct mapInfo {
	void* map_addr;
	size_t size;
};
std::vector<struct mapInfo> mmapBuf;


private:
	uint64_t getTimeUs(void);
	uint64_t getTimeUsFromStart(void);

	/* Map: bitstreamId -> timestamp */
	std::map<int32_t, uint64_t> mInputWork;

	std::map<int32_t, uint8_t*> mInputBuffer;

	/* Map: slot -> picturebufferId */
	std::map<int32_t, int32_t> mOutputBufferPictureId;

	/* Queue: bistreamId -> struct dispwork */
	std::queue<dispWork> mDisplayWork;
	/* output buffer mmap addr */
	std::vector<void*>mMmapAddr;

	std::mutex mInputLock;
	std::mutex mOutputLock;
	std::mutex mDumpLock;

	uint32_t mDqWidth;
	uint32_t mDqHeight;
	uint32_t mInputDoneCount;
	uint32_t mOutputDoneCount;
	uint32_t mOutputBufferNum;
	uint32_t mDumpNum;
	uint64_t mStartTime;
	uint32_t mbitstreamId;
};

uint64_t VideoDecPlayerExample::getTimeUs(void) {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1e6 + time.tv_usec;
}

uint64_t VideoDecPlayerExample::getTimeUsFromStart(void) {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1e6 + time.tv_usec - mStartTime;
}

int VideoDecPlayerExample::init(uint32_t vFmt, uint32_t width, uint32_t height, uint32_t framerate) {
	int ret = 0;

	init_param_t config = {
		/* video */ 	256, width, height, framerate, vFmt, 0,
		/* audio */ 	0, 2, 44100, 2,
		/*pcrid */		0,
		/* display */	1, 0, 0, 0,
						2, 0, 0, 0, 0, 0, 0};

	mAmVideoDec->setQueueCount(kDefaultQueueCount);
	ret = mAmVideoDec->initialize(vformat_to_mime(vFmt), (uint8_t*)&config, sizeof(init_param_t), false, true);
	if (ret) {
		printf("init failed!, ret=%d\n", ret);
		return -2;
	}

	return 0;
}

int VideoDecPlayerExample::inputBuffer(uint8_t* buf, uint32_t size, uint64_t timestamp) {
	if (buf == NULL || size == 0) {
		printf("please input valid data\n");
		return -1;
	}

	mInputBuffer[mbitstreamId] = buf;

	int retry = 500;
	int32_t err = 0;
	do {
		err = mAmVideoDec->queueInputBuffer(mbitstreamId, mInputBuffer[mbitstreamId], 0, size, timestamp);
		if (err == -EAGAIN) {
			usleep(10000);
		} else {
			break;
		}
	} while(err || retry-- > 0);
	std::lock_guard<std::mutex> lock(mInputLock);
	mInputWork[mbitstreamId] = timestamp;
	mbitstreamId++;

	return 0;
}

void VideoDecPlayerExample::dumpData(char* buf, uint32_t width, uint32_t height, int32_t bitstreamId) {
	std::lock_guard<std::mutex> lock(mDumpLock);
	uint32_t y_size = (width* height);
	uint32_t uv_size = (width* height) / 2;

	if (moFp) {
		/* save output yuv data */
		fwrite(buf, y_size, 1, moFp);
		fwrite(buf + (mDqWidth* mDqHeight), uv_size, 1, moFp);
		fflush(moFp);
		fsync(fileno(moFp));

		printf("Dump YUV y_size %d,uv_size %d bitstreamId %d\n",
			y_size, uv_size, bitstreamId);
	}
	if (mcFp) {
		/* save output crc data */
		int crc_y = 0,crc_uv = 0;

		crc_y = crc32_le(crc_y, (unsigned char const *)buf, y_size);
		crc_uv = crc32_le(crc_uv, (unsigned char const *)(buf + mDqWidth* mDqHeight), uv_size);

		fprintf(mcFp, "%08d: %08x %08x\n", mDumpNum, crc_y, crc_uv);
		fflush(mcFp);
		fsync(fileno(mcFp));
		printf("%08d: %08x %08x\n", mDumpNum, crc_y, crc_uv);
	}
	mDumpNum++;
}


void VideoDecPlayerExample::outputBuffer(const char* oname, int num) {
	int32_t bitstreamId = 0;
	int64_t timestamp = 0;
	int32_t pictureBufferId = 0;
	uint32_t width = 0;
	uint32_t height = 0;

	if (mDisplayWork.empty()) {
		usleep(5000);
		return;
	}
	std::lock_guard<std::mutex> lock(mOutputLock);

	dispWork* work = &mDisplayWork.front();
	if (work == NULL) {
		return ;
	}

	bitstreamId = work->bitstreamId;
	timestamp = work->timestamp;
	pictureBufferId = work->pictureBufferId;
	width = work->width;
	height = work->height;

	if (oname != NULL && moFp == NULL && mcFp == NULL) {
		char newoname[128];
		char newcname[128];

		sprintf(newoname, "%s_%d_%d_%d.yuv", oname, num, width, height);
		sprintf(newcname, "%s_%d_%d_%d.crc", oname, num, width, height);
		moFp = fopen(newoname, "wb");
		if (!moFp) {
			printf("Unable to open output YUV file\n");
			return;
		}
		setbuf(moFp,NULL);

		mcFp = fopen(newcname, "w");
		if (!mcFp) {
			printf("Unable to open output crc file\n");
			return;
		}
		setbuf(mcFp,NULL);
	}

	char* vaddr = (char *)mMmapAddr[pictureBufferId];

	dumpData(vaddr, width, height, bitstreamId);

	mAmVideoDec->queueOutputBuffer(pictureBufferId);

	mDisplayWork.pop();
}

AmVideoDecBase* getAmVideoDec(AmVideoDecCallback* callback) {
	void *libHandle = dlopen("libmediahal_videodec.so", RTLD_NOW);

	if (libHandle == NULL) {
		libHandle = dlopen("libmediahal_videodec.system.so", RTLD_NOW);
		if (libHandle == NULL) {
			printf("unable to dlopen libmediahal_videodec.so: %s\n", dlerror());
			return nullptr;
		}
	}

	typedef AmVideoDecBase *(*createAmVideoDecFunc)(AmVideoDecCallback* callback);

	createAmVideoDecFunc getAmVideoDec =
		(createAmVideoDecFunc)dlsym(libHandle, "AmVideoDec_create");

	if (getAmVideoDec == NULL) {
		dlclose(libHandle);
		libHandle = NULL;
		printf("can not create AmVideoDec\n");
		return nullptr;
	}
	AmVideoDecBase* halHanle = (*getAmVideoDec)(callback);
	printf("getAmVideoDec ok\n");
	return halHanle;
}

VideoDecPlayerExample::VideoDecPlayerExample() {
	moFp = nullptr;
	miFp = nullptr;
	mcFp = nullptr;
	msFp = nullptr;
	mStartTime = getTimeUs();
	mInputDoneCount = 0;
	mOutputDoneCount = 0;
	mOutputBufferNum = 0;
	mDumpNum = 0;
	mready = false;
	out_flag = false;
	mbitstreamId = 0;
	mCallback = new playerCallback(this);
	mAmVideoDec = getAmVideoDec(mCallback);
}

VideoDecPlayerExample::~VideoDecPlayerExample() {
	delete mCallback;
	delete mAmVideoDec;

	if (miFp) {
		fclose(miFp);
	}
	if (msFp) {
		fclose(msFp);
	}
	if (moFp) {
		fclose(moFp);
	}
	if (mcFp) {
		fclose(mcFp);
	}

	std::map<int32_t, uint8_t*>::iterator iter;
	iter = mInputBuffer.begin();
	while (iter != mInputBuffer.end()) {
		free(iter->second);
		iter->second = NULL;
		iter++;
	}
}


void VideoDecPlayerExample::play(const char* iname, const char* sname, const char* oname, int num) {
	int end = 0;
	int ret = 0;

	if (iname == NULL) {
		printf("iname is null\n");
		return ;
	} else {
		miFp = fopen(iname, "rb");
		if (!miFp) {
			printf("open input file error %s!\n",iname);
			return;
		}
	}

	if (sname == NULL) {
		printf("sname is null\n");
		return ;
	} else {
		msFp = fopen(sname, "rb");
		if (!msFp) {
			printf("open frame size file error!\n");
			return;
		}
	}

	printf("\n*********VideoDec PLAYER DEMO************\n\n");
	printf("file %s to be played\n", iname);

	std::atomic_bool running(true);
	std::thread outputThread([this, &running, oname, num]() {
		while (running) {
			outputBuffer(oname, num);
		}
	});

	while (1) {
		char frame_size_str[32];
		char *s_rt;
		uint32_t frame_size;
		uint8_t * newbuf;

		if (mInputBuffer.size() > 50) {
			printf("sleep 20000\n");
			usleep(20000);
		}

		memset(frame_size_str, 0, sizeof(frame_size_str));
		s_rt = fgets(frame_size_str, 32, msFp);

		if (s_rt == NULL) {
			break;
		}
		frame_size = atoi(frame_size_str);

		if (frame_size <= InputBufferMaxSize) {
			newbuf = (uint8_t *)malloc(frame_size);
			if (newbuf == NULL) {
				printf("malloc frame size %d fail\n", frame_size);
			}
			memset(newbuf, 0, frame_size);
			ret = fread(newbuf, 1, frame_size, miFp);
			printf("read size %d, %x %x %x %x %x %x %x %x ...\n", frame_size,
				newbuf[0], newbuf[1], newbuf[2], newbuf[3], newbuf[4], newbuf[5], newbuf[6], newbuf[7]);

			if (ret < frame_size) {
				printf("read back size %d, less than frame size %d\n", ret, frame_size);
				return ;
			}
		} else {
			printf("Error:input frame size(%d) is error!\n",frame_size);
			return;
		}

		ret = inputBuffer(newbuf, frame_size, 0);
		if (ret) {
			printf("Error:input buffer is error!\n");
			return;
		}

		if (feof(miFp) || feof(msFp)) {
			end = 1;
		}
		if (end) {
			break;
		}
	}

	mAmVideoDec->flush();

	std::unique_lock <std::mutex> l(mFlushedLock);
	while (!mready) {
		mFlushedCondition.wait(l);
	}

	while ((mDisplayWork.size() > 0)) {
		usleep(10000);
	}

	running.store(false);
	outputThread.join();
	mAmVideoDec->destroy();
}

void VideoDecPlayerExample::onOutputFormatChanged(uint32_t requestedNumOfBuffers,
				int32_t width, uint32_t height) {

	printf("onOutputFormatChanged bufnum %d, width %d, height %d\n",
					requestedNumOfBuffers, width, height);

	/* resolution change clean resource */
	if (mOutputBufferNum > 0) {
		printf("resolution change free all buffer, mOutputBufferNum %d\n", mOutputBufferNum);
#if ANDROID_PLATFORM_SDK_VERSION >= 30
		mAmVideoDec->freeUvmBuffers();
#else
		mAmVideoDec->freeAllIonBuffer();
#endif
	}


#if ANDROID_PLATFORM_SDK_VERSION >= 30
	mOutputBufferNum = requestedNumOfBuffers;
	mDqWidth = width;
	mDqHeight = height;
	uint32_t imagesize = (width * height * 3) / 2;
	mAmVideoDec->setupOutputBufferNum(mOutputBufferNum);
	for (uint32_t i = 0; i < mOutputBufferNum; i++) {
		uint8_t* vaddr;
		int fd;
		mAmVideoDec->allocUvmBuffer(width, height, (void**)&vaddr, i, &fd);
		mMmapAddr.push_back((void*)vaddr);
		printf("allocUvmBuffer fd %d, size = %d\n", fd, imagesize);
		mAmVideoDec->createOutputBuffer(i, fd);
	}
#else
	mOutputBufferNum = requestedNumOfBuffers;
	mDqWidth = width;
	mDqHeight = height;
	uint32_t imagesize = (width * height * 3) / 2;
	mAmVideoDec->setupOutputBufferNum(mOutputBufferNum);

	for (uint32_t i = 0; i < mOutputBufferNum; i++) {
		uint8_t* vaddr;
		int fd;

		mAmVideoDec->allocIonBuffer(imagesize, (void**)&vaddr, &fd);
		mMmapAddr.push_back((void*)vaddr);
		printf("allocIonBuffer fd %d, size = %d\n", fd, imagesize);
		mAmVideoDec->createOutputBuffer(i, fd);
	}
#endif

	printf("onOutputFormatChanged out timeUs %lld\n", getTimeUsFromStart());
}


void VideoDecPlayerExample::onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
				uint32_t width, uint32_t height) {
	printf("onOutputBufferDone this %p, pictureBufferId %d, bitstreamId %lld, mInputWork.size %d, In %d, Out %d\n",
				this, pictureBufferId, bitstreamId, mInputWork.size(), mInputDoneCount, mOutputDoneCount);
	std::lock_guard<std::mutex> lock(mOutputLock);
	int64_t timestamp = mInputWork[bitstreamId];
	printf("onOutputBufferDone this %p, timestamp %lld, width %d, height %d\n",this, timestamp, width, height);

	mDisplayWork.push({bitstreamId, timestamp, pictureBufferId, width, height});
	mInputWork.erase(bitstreamId);
	mOutputDoneCount++;
}

void VideoDecPlayerExample::onInputBufferDone(int32_t bitstreamId) {
	printf("%s bitstream_buffer_id:%d\n", __func__, bitstreamId);
	std::lock_guard<std::mutex> lock(mInputLock);
	mInputBuffer.erase(bitstreamId);
	free(mInputBuffer[bitstreamId]);
	mInputDoneCount++;
}

void VideoDecPlayerExample::onUpdateDecInfo(const uint8_t* info, uint32_t isize) {
	printf("%s info:%s isize:%d\n", __func__, info, isize);
}

void VideoDecPlayerExample::onFlushDone() {
	printf("onFlushDone\n");
	std::unique_lock <std::mutex> l(mFlushedLock);
	mready = true;
	mFlushedCondition.notify_all();
}

void VideoDecPlayerExample::onResetDone() {
	printf("onResetDone\n");
}

void VideoDecPlayerExample::onError(int32_t error) {
	printf("%s error%d\n", __func__, error);
}

void VideoDecPlayerExample::onEvent(uint32_t event, void* param, uint32_t paramsize) {
	UNUSED(param);
	UNUSED(paramsize);
	printf("%s event:%x, %s %d\n", __func__, event, (char*)param, paramsize);
}

static void usage(void)
{
	printf("VideoDecPlayer\n");
	printf("Usage: VideoDecPlayerExample -i <file> -f <format> -s <frame size> [-n <number>] [-o <out yuv and crc>] [-h <help>]\n");
	printf("\n");
	printf(" -i, --ifile		input es file\n");
	printf(" -f, --format		video format\n");
	printf ("\t\t0:mpeg12\t1:mpeg4\t\t2:h264\t\t3:mjpeg\n\
		5:jpeg\t\t6:vcl\t\t7:avs\t\t11:hevc\n\
		14:vp9\t\t15:avs2\t\t16:av1\n");
	printf(" -s, --size 	frame size file\n");
	printf(" -n, --number	instance number\n");
	printf(" -o, --output	output yuv and crc\n");
	printf(" -h, --help	usage\n");
}


int main(int argc, char** argv) {
	int optionChar = 0;
	int optionIndex = 0;
	uint32_t   vFmt = -1;
	char* iname = nullptr;
	char* sname = nullptr;
	char* oname = nullptr;
	int num = 1;

	if (argc < 7) {
		usage();
		return -1;
	}

	const char *shortOptions = "i:f:s:n:o:h";
	struct option longOptions[] = {
		{ "ifile", required_argument, nullptr, 'i' },
		{ "format", required_argument, nullptr, 'f' },
		{ "size", required_argument, nullptr, 's' },
		{ "number", required_argument, nullptr, 'n' },
		{ "output", required_argument, nullptr, 'o' },
		{ "help", no_argument, nullptr,  'h'},
		{ nullptr, 0, nullptr, 0 },
	};

	while ((optionChar = getopt_long(argc, argv, shortOptions,
			longOptions, &optionIndex)) != -1) {
		switch (optionChar) {
		case 'i':
			iname = optarg;
			break;
		case 'f':
			vFmt = atoi(optarg);
			break;
		case 's':
			sname = optarg;
			break;
		case 'n':
			num = atoi(optarg);
			break;
		case 'o':
			oname = optarg;
			break;
		case 'h':
			usage();
			exit(-1);
		default:
			break;
		}
	}

	if (iname == nullptr || vFmt == -1 || sname == nullptr) {
		usage();
		exit(-1);
	}

	if (num > MAX_INSTANCE_MUN) {
		printf("input instance num is more than 9\n");
		exit(-1);
	}

	sysfs_cmd_control("/sys/module/amvdec_ports/parameters/bypass_vpp", "1");

	std::vector<std::thread> threads;

	for (int i = 1 ; i <= num ; i++) {
		threads.push_back(std::thread([iname, sname, oname, vFmt, i]() {
			printf("start player %d\n", i);
			VideoDecPlayerExample player;
			player.init(vFmt);
			printf("iname %s,sname %s,vFmt = %d\n",iname,sname,vFmt);
			player.play(iname,sname,oname,i);
		}));
	}

	for (auto& thread : threads) {
        thread.join();
    }

	printf("VideoDecPlayerExample exit\n");
	sysfs_cmd_control("/sys/module/amvdec_ports/parameters/bypass_vpp", "0");

	return 0;
}

