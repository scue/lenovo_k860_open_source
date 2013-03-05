#ifndef _S3CFB_H
#define _S3CFB_H 

struct s3cfb_lcd_timing {	
	int	h_fp;
	int	h_bp;
	int	h_sw;
	int	v_fp;
	int	v_fpe;
	int	v_bp;
	int	v_bpe;
	int	v_sw;
	int	cmd_allow_len;
};

struct s3cfb_lcd_polarity {
	int rise_vclk;
	int inv_hsync;
	int inv_vsync;	
	int inv_vden;
};

struct s3cfb_lcd {
	int	width;
	int	height;
	int	bpp;
	int	freq;
	struct	s3cfb_lcd_timing timing;
	struct	s3cfb_lcd_polarity polarity;
	void	(*init_ldi)(void);
	void	(*deinit_ldi)(void);
};


#endif