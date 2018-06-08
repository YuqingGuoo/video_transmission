采集摄像头数据 用FFMPEG转码H264格式，取消了main函数里的线程，已成功转换，但是在tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf)时，
tVideoBuf数据有变动。此BUG暂时没有解决
