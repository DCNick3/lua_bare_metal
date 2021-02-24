#include <arch/i386/peripheral.h>
#include <arch/i386/isa_dma.h>
#include <stdint.h>
#include <procio.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

/* Floppy controller driver. Supports only 3.5" HD 1440 KiB disks */

#define PORT 0x3F0

//#define SRA (PORT+0)   //Status register A
//#define SRB (PORT+1)   //Status register B

#define FDC_DOR (PORT+2)   //Digital output register
//#define FDC_TDR (PORT+3)   //Tape drive register
#define FDC_MSR (PORT+4)   //Main state register
#define FDC_DRS (PORT+4)   //Datarate select register
#define FDC_DATA (PORT+5)  //Data FIFO

#define FDC_DIR (PORT+7)   //Digital input register
#define FDC_CCR (PORT+7)   //Configuration control register

/* DOR flags */
#define DOR_IRQ   0x08
#define DOR_RESET 0x04

/* MSR flags */
#define MSR_RQM  0x80
#define MSR_DIO  0x40
#define MSR_NDMA 0x20
#define MSR_CB   0x10

#define MSR_ACTD 0x08
#define MSR_ACTC 0x04
#define MSR_ACTB 0x02
#define MSR_ACTA 0x01

#define MSR_OK_READ (MSR_RQM | MSR_DIO | MSR_CB)

/* DIR flags */
#define DIR_DC 0x80


#define SECTOR_SIZE 512
#define SEC_PER_TRACK 18 

#define MS 1000

#define FL_HLT (8*MS)
#define FL_SPIN_UP (400*MS)
#define FL_SPIN_DN (3000*MS)
#define FL_SEL_D (20*MS)
#define FL_INT_TMT (30000*MS)

#define CMD_SPECIFY  0x03
#define CMD_WRITE    0x45
#define CMD_READ     0x46
#define CMD_SENSEI   0x08
#define CMD_READID   0x4A
#define CMD_RECAL    0x07
#define CMD_SEEK     0x0F
#define CMD_VERSION  0x10
#define CMD_CONF     0x13

#define WAIT_COND(c) while (!(c)) {}

#define dbg_log(...) printf(__VA_ARGS__);

#define DMA_CHANNEL 2

typedef struct {
	uint16_t track;
	int motstate;
	int dchg;
} drive_state_t;

static volatile int irq_signaled = 0;
static uint8_t reg_dor, reg_sr0, reg_sr1, reg_sr2;

static drive_state_t drives[4];
static uint8_t cur_drive = 0;
static int motoff_timer;

static int status_sz;
static uint8_t status[7];

static void* dma_buffer;

static void lba_2_chs(uint32_t lba, uint16_t *c, uint16_t *h, uint16_t *s)
{
	*c = lba / (2 * SEC_PER_TRACK);
	*h = ((lba % (2 * SEC_PER_TRACK)) / SEC_PER_TRACK);
	*s = ((lba % (2 * SEC_PER_TRACK)) % SEC_PER_TRACK + 1);
}

static int sendbyte(uint8_t d)
{
	volatile uint8_t msr;
	for (int i = 0; i < 128*128; i++)
	{
		msr = inb(FDC_MSR);
		if (msr & MSR_RQM && !(msr & MSR_DIO))
		{
			outb(FDC_DATA, d);
			return 0;
		}
		io_wait();
	}
	return -1; /* timeout */
}

static int getbyte()
{
	volatile uint8_t msr;
	for (int i = 0; i < 128*128; i++)
	{
		msr = inb(FDC_MSR);
		if ((msr & MSR_OK_READ) == MSR_OK_READ)
		{
			return inb(FDC_DATA);
		}
		io_wait();
	}
	return -1; /* timeout */
}

static int fdc_waitint()
{
	clock_t s = clock();
	int tmout;
	WAIT_COND(!irq_signaled && (tmout = (clock() - s < FL_INT_TMT)));
	if (!tmout)
		return -1;
	return 0;
}

static int fdc_wait(int sensei)
{
	int tmout = fdc_waitint() | fdc_waitint();
	
	status_sz = 0;
	while (status_sz < 7 && (inb(FDC_MSR) & MSR_RQM))
		status[status_sz++] = getbyte();

	if (sensei)
	{
		sendbyte(CMD_SENSEI);
		reg_sr0 = getbyte();
		drives[cur_drive].track = getbyte();
	}

	irq_signaled = 0;
	if (tmout)
	{
		if (inb(FDC_DIR) & DIR_DC)
			drives[cur_drive].dchg = 1;
		return 1;
	}
	else
		return 0;
}

static void motor_on()
{
	if (drives[cur_drive].motstate == 0)
	{
		reg_dor |= (1 << (cur_drive + 4));
		outb(FDC_DOR, reg_dor);
		drives[cur_drive].motstate = 1;
		usleep(FL_SPIN_UP);
	}
}

static void motor_off()
{
	if (drives[cur_drive].motstate)
	{
		motoff_timer = 2000;
	}
}

static void fdc_configure()
{
	sendbyte(CMD_CONF);
	sendbyte(0x00);
	sendbyte(0x17); /* threshold = 8, FIFO on, polling off, implied seek on */
	sendbyte(0x00);
}

static void floppy_specify()
{
	sendbyte(CMD_SPECIFY);
	sendbyte((0x08 << 4) | (0x5)); /* SRT=8 and HUT=5 */
	sendbyte((0x05 << 1) | (0x0)); /* HLT=5 and NDMA=0 */
}

static void fdc_reset()
{
    dbg_log("doing FDC reset\n");

    irq_signaled = 0;

	/* Send reset */
	outb(FDC_DOR, 0x00);
	io_wait();
	reg_dor = DOR_IRQ | DOR_RESET;
	outb(FDC_DOR, reg_dor);
	io_wait();

	/* Wait for IRQ or timeout */
    if (fdc_waitint())
		dbg_log("FDC irq timeout\n"); /* Hmph... Timeout */

    
	for (int i = 0; i < 4; i++)
	{
		sendbyte(CMD_SENSEI);
		reg_sr0 = getbyte();
		drives[i].track = getbyte();
	    //dbg_log("SENSEI after RST; sr0=0x%x track=%d\n", reg_sr0, drives[i].track);
    }

	irq_signaled = 0;

	fdc_configure();
    floppy_specify();
}

static int floppy_select(int drive)
{
	if (drive < 0 || drive > 3)
	{
		return -1;
	}
	
	outb(FDC_CCR, 0x00); /* 1.44 MiB floppy */

	for (int i = 0; i < 4; i++) drives[i].motstate = 0;

	cur_drive = (uint8_t)drive;
	reg_dor = (reg_dor & 0x0C) | cur_drive;  /* stop all motors and select new drive
                                                will save only IRQ and RESET flags*/
	outb(FDC_DOR, reg_dor);

	floppy_specify();
	
	return 0;
}

static void floppy_calibrate()
{
	motor_on();

	sendbyte(CMD_RECAL);
	sendbyte(cur_drive);

	fdc_wait(1);

	motor_off();
}

static int floppy_seek(unsigned track)
{
	if (drives[cur_drive].track == track)
		return 0;

	int res = 0;
	
	motor_on();

	sendbyte(CMD_SEEK);
	sendbyte(cur_drive);
	sendbyte((uint8_t)track);
	
	if (fdc_wait(1))
		res = 1;

	usleep(15*MS);

	motor_off();

	if (!(reg_sr0 & 0x20) || (drives[cur_drive].track != track))
		return 1;
	else
		return res;
}

static int floppy_present()
{
	uint8_t dir = inb(FDC_DIR);
	if (dir & DIR_DC)
	{
		floppy_seek(79);
		floppy_seek(0);
	}
	return !(inb(FDC_DIR) & DIR_DC);
}

static int floppy_transfer_try(uint8_t* buf, int do_write, int cnt, uint16_t c,  uint16_t h, uint16_t s)
{
	uint8_t cmd = do_write ? CMD_WRITE : CMD_READ;

    if (do_write)
        memcpy(dma_buffer, buf, cnt * SECTOR_SIZE);

    isa_dma_init_transfer(DMA_CHANNEL, dma_buffer, cnt * SECTOR_SIZE - 1, do_write, 0, 0, ISA_DMA_MODE_SINGLE);

	irq_signaled = 0;

	sendbyte(cmd);
	sendbyte((uint8_t)((h << 2) | cur_drive));
	sendbyte((uint8_t)c);
	sendbyte((uint8_t)h);
	sendbyte((uint8_t)s);
	sendbyte(0x2);
	sendbyte((uint8_t)80); /* TODO: de-hardcode it */
	sendbyte(0x1b);
	sendbyte(0xff);

	int tmout = fdc_wait(0);

    if (!tmout && !do_write)
    {
        memcpy(buf, dma_buffer, cnt * SECTOR_SIZE);
    }

	reg_sr0 = status[0];
	reg_sr1 = status[1];
	reg_sr2 = status[2];
	drives[cur_drive].track = status[3];
	//getbyte(); getbyte();
	//getbyte(); /*secsize*/
	
    return tmout;
}

static int floppy_transfer(uint8_t *buf, int do_write, int cnt, uint16_t c, uint16_t h, uint16_t s)
{
	int res = -1;
	for (int i = 0; i < 4; i++)
	{
		floppy_seek(c);
		if (!(res = floppy_transfer_try(buf, do_write, cnt, c, h, s)))
		{
			if (!(reg_sr0 & 0xC0))
			{
				printf("read %d sectors\n", cnt);
				break;
			}
			if (do_write && (reg_sr1 & 0x2))
			{
				res = -3;
			}
		}
		printf("transfr fail; msr=0x%x, sr0=0x%x, sr1=0x%x, sr2=0x%x, trk=0x%x\n", inb(FDC_MSR), reg_sr0, reg_sr1, reg_sr2, drives[cur_drive].track);
		floppy_calibrate();
		if (i == 1)
		{
			fdc_reset();
			floppy_select(cur_drive);
			motor_on();
		}
	}
	return res;
}

static int floppy_transfer_lba(uint8_t *buf, uint32_t lba, uint16_t num, int do_write)
{
	motor_on();

	int res = 0;

	for (int i = 0; i < num; i++)
	{
		uint16_t c, h, s;
		lba_2_chs(lba, &c, &h, &s);

		res = floppy_transfer(buf, do_write, 1, c, h, s);
		lba++;
	}

	motor_off();

	return res;
}

int floppy_read(const uint8_t *buf, uint32_t offset, uint32_t num)
{
	return floppy_transfer_lba((uint8_t*)buf, offset, (uint16_t)num, 0);
}

int floppy_write(uint8_t *buf, uint32_t offset, uint32_t num)
{
	return floppy_transfer_lba(buf, offset, (uint16_t)num, 1);
}

void floppy_init()
{
    dma_buffer = isa_dma_alloc_buffer();
    printf("floppy got dma buffer at 0x%x\n", dma_buffer);

	fdc_reset();
	sendbyte(0x10);
	int r = getbyte();
	printf("fdc ver = 0x%x; sr0 = 0x%x; track = 0x%x\n", r, reg_sr0, drives[cur_drive].track);
	if (r != 0x90)
	{
		printf("floppy might not work correctly\n");
	}

	printf("select\n");
	floppy_select(0);
	printf("calibrate\n");
	floppy_calibrate();
	printf("done; sr0 = 0x%x\n", reg_sr0);

	int pres = floppy_present();
	printf("floppy_present() = 0x%x\n", pres);
	uint8_t buf[512];
	printf("read(0)  = 0x%x\n", floppy_read(buf, 0, 1));
	for (int i = 0; i < 64; i++)
        printf("%3x", buf[i]);
    printf("\n");
    printf("write(1) = 0x%x\n", floppy_write(buf, 1, 1));
}

/* this is IRQ6 handler */
void floppy_isr()
{
	irq_signaled = 1;
}


/* this's being called from timer isr (1000 Hz) */
void floppy_timer()
{
	if (motoff_timer > 0)
	{
		motoff_timer--;
		if (motoff_timer == 0)
		{
			reg_dor &= ~(1 << (cur_drive + 4));
			outb(FDC_DOR, reg_dor);
			drives[cur_drive].motstate = 0;
		}
	}
}
