#pragma once
#include "kac12040_def.h"
#define FIXED_FRAME_MODE 0
#define CONTINUOUS_FRAME_MODE 1

#define FLASH_ERASE_ST 1
#define FLASH_WRITE_ST 2
#define FLASH_IDLE_ST 0

#define CFG_BIN_SIZE 2453372

void setUsbHandle(PVOID handle);

int init_camera(void);
int CamRegWrite(unsigned Addr, unsigned data);
int CamRegRead(unsigned Addr);
unsigned CalcChkSum(unsigned *cmd);
int CmdSensorRegRead(unsigned Addr);
int CmdSensorRegWrite(unsigned Addr, unsigned data);
int CamSetROI(void);
int CamSetCaptureParam(void);
int CamSetTrigExposure(void);
int CamSetSensorGain(void);
int CamFrameRequest(void);
int CamStrbParam(void);
int grab_start(void);
int grab_stop(void);
int CheckRxPacket(unsigned *buf, int len);
int SensorRegRead(unsigned int addr, unsigned int *RdData);
int GetSensorID(unsigned int *SensorID);
void SetSensorFLOTiming(void);
void SetSensorMode(unsigned shutterMode, unsigned flip);
int CmdFlushImagePipe(unsigned FlushOption);
void FlushImagePipe(void);
int GetCameraVersion(float *version);
int FpgaRegRead(unsigned int addr, unsigned int *RdData);
void DisableCompanding(void);
void SetCompandingMode(void);

int GetDefaultCmdAck(unsigned cmd_type);
int DDR2_Memory_Write(unsigned cam_mem_addr, unsigned cam_memc, unsigned size_bytes, unsigned *Data);
int DDR2_Memory_Read(unsigned cam_mem_addr, unsigned cam_memc, unsigned size_bytes, unsigned *Data);
int Get_DDR2_read_response(unsigned data_byte_size, unsigned *Data);
int ConfigFileDownload(char *src_buf, char *verify_buf, unsigned data_size_byte);

int ProgramSPI_Flash(char *src_buf, char *verify_buf, int cam_mem_addr, int data_size_byte);
int Cmd_SPI_FlashWrite(unsigned src_Addr, unsigned data_size_bytes);
int Cmd_SPI_FlashRd(unsigned dst_Addr, unsigned data_size_bytes);
int CheckFlashStatus(void);
int Get_SPI_Flash_StatusCmdAck(unsigned *OpStatus, unsigned *ProgressCnt);
int Cmd_Get_SPI_Flash_OpStatus(void);
unsigned int CalcDefectMapChksum(int defect_size, defect_pixel_t* defect_pixel);
int DownloadToCameraMemory(char *src_buf, char *verify_buf, unsigned data_size_byte);
int GetCmdAckWstatus(unsigned cmd_type);
int Cmd_SPI_FlashDirectRd(unsigned pktid,unsigned flash_rd_Addr, unsigned data_size_bytes);
int Get_SPI_Flash_Direct_read_response(unsigned data_byte_size, unsigned *Data);
int Cmd_DefectMap_Write(unsigned defect_size, unsigned defect_map_chksum);
int GetDefectMap(defect_map_t* defect_map);
int SaveDefectMapToCam(int defect_size, defect_map_t* defect_map);