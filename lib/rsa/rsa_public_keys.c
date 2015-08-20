#include <common.h>
#include <stdint.h>
#include <rsa_public_key.h>
#include <board_id.h>

#define MAX_BOARD_ID	SPACECAST_BOARD_ID

static int board_id_to_key_index[] = {
	0, /* OPTIMUS_BOARD_ID */
	0, /* SIDESWIPE_BOARD_ID */
	1, /* SPACECAST_BOARD_ID */
	-1, /* UNASSIGNED_BOARD_ID */
	0, /* OPTIMUS_PRIME_BOARD_ID */
	0, /* SIDESWIPE_PRIME_BOARD_ID */
};

/*
 * The keys are generated from generate_public_key_code.c
 */
#ifndef CONFIG_COMCERTO_BOOTLOADER
/* Keys to verify the signature of Barebox */
static const struct rsa_public_key public_keys[] = {
	/* Optimus / Sideswipe */
	{ .n0inv = 324625445u,
	  .modulus = {
			0xdd47e853, 0xcc40a072, 0xccc006f4, 0xc941d763, 0x8f53aa97, 0x26f1e34a,
			0xb476f79a, 0xf4aaaaaa, 0x676ae350, 0xd8b0d648, 0x18fc6347, 0x984bb7d3,
			0xa894b4a0, 0x51ea64c3, 0xeda9ea3c, 0xdcff20c7, 0x88afc95f, 0xa485fce7,
			0xd7010b97, 0x6e821430, 0xf836bbda, 0x004b61fe, 0xcddcc51a, 0x8e2f01f0,
			0x92647f40, 0xb60cc2e2, 0x570c6692, 0x40e51f0a, 0x34e9036f, 0x6b56ad55,
			0xe4d044e4, 0x2d32bd77, 0xfed334b5, 0x7505311f, 0x98f0fc46, 0xa557619e,
			0xb1158093, 0xf3a785c1, 0xf6e77d38, 0xe0e29c9b, 0x3aba8b7f, 0x212ee1f4,
			0x636549c6, 0x5bc9597b, 0xf2f35938, 0xf0ab94b7, 0x63519a73, 0x62dd79e6,
			0x3e0863f8, 0xf9d67dab, 0x2dbf66b7, 0x45bb69e6, 0xdc3836bb, 0xd253d7f4,
			0x975d5f43, 0x40eac67c, 0xd27139fa, 0xb2c73104, 0xced1c1f2, 0x7353f693,
			0xee23ed71, 0xc710fdf4, 0x330f3b81, 0xb4a49e97
		},
	  .rr = {
			0xc416ec5a, 0x8f35e335, 0x7860fdb6, 0xe3dd9018, 0xb021e1a8, 0x950621cb,
			0xb4d3c3e5, 0xe52c0288, 0x25366c47, 0xae481186, 0x8f4f42e4, 0x5203857d,
			0x60517eb4, 0xa49474c0, 0x952bdc12, 0xbdba6b4b, 0xac90fe1d, 0xbf7e29f9,
			0x41fb4116, 0xc2b9ca56, 0xf48e1cbd, 0xe7f43887, 0xebdf65f6, 0x694773f6,
			0x1efd33be, 0x4d7c0c9d, 0xd01b12f9, 0xde65788a, 0x93db7132, 0x8a8e23b1,
			0x0faab0b6, 0x7fd78d30, 0xe7e04e64, 0xdd0b2eab, 0xa93630d2, 0x07928be0,
			0x4ff324a5, 0x494b52da, 0xbed79044, 0x93d5c5b6, 0xdfb6b89b, 0x3c0ad0f5,
			0xdfaf19b6, 0x07d4e67f, 0xba2fc62a, 0x58697a86, 0x1c973e7f, 0x66a26c99,
			0x45a0ded5, 0x4f69d633, 0xbe326828, 0xd9d31655, 0x699b6a54, 0x2cb1ee8b,
			0x69dcbbc0, 0x9469b4fa, 0xf3b23e6b, 0x886ec86a, 0x2e9410a9, 0xa41074cf,
			0xa4c4ca4f, 0xd3ec5963, 0xf36f1929, 0x22bf4a0b
		}
	},

	/* SpaceCast */
#ifdef CONFIG_RSA_KEY_SET_DEFAULT
	{ .n0inv = 810249397u,
	  .modulus = {
			0x6a50de63, 0x3040dda3, 0xb055e6ac, 0x6bf8de72, 0xa4a5ab53, 0x84edb6d8,
			0x52fb91aa, 0xb3b17524, 0x7ced643f, 0x887d958f, 0x7e9c74fe, 0xb0501334,
			0x27f00dea, 0x2d1521c5, 0xc6ba2dd3, 0x573e8cce, 0x56a69f52, 0x284bee21,
			0x0cb26d61, 0x2c336a09, 0x143f9d44, 0x928c224f, 0x5468a697, 0x714b716a,
			0x17760fcb, 0xba3a0e45, 0x05ecd6de, 0x5698fad1, 0x5fe3f42b, 0xd81d59bf,
			0xb1aa588b, 0x954b54bf, 0x149121cb, 0xb1137ff8, 0x48fe0df8, 0x9c4d3a1f,
			0x7ab01207, 0x5090b948, 0xb6438e5d, 0x407aee2e, 0x4f9806e4, 0xf5789b89,
			0x03894a74, 0x00326f7c, 0x3798956c, 0xc0d34857, 0xb9c4a166, 0x525f7968,
			0x2f7172f6, 0xb45071f5, 0x6dead442, 0x3fcbfc1f, 0x44308880, 0xba7e3fd8,
			0xb802d128, 0x786ecc50, 0x2af891f6, 0x80685385, 0xc8695ed2, 0x3ac79fd3,
			0xaa18dcc3, 0x75b67ea1, 0xf6ebfd40, 0xd81f9abd
		},
	  .rr = {
			0xe57d1491, 0x314d95bb, 0xd1934a17, 0x97ff927d, 0x538863de, 0xab32e57b,
			0x1d293896, 0x96203ed5, 0xe800e4f4, 0xdc39472d, 0x2b86e9f5, 0x7fd34b1c,
			0x941bfd55, 0x7d29bf70, 0xaa23fcbc, 0x76004c4a, 0x2b2c49b5, 0x2cbfd2ac,
			0x35fa4ac4, 0x58ce6253, 0xa1776240, 0x3b8a88a7, 0xf3e02089, 0xfcc8b094,
			0xa64531b1, 0x738e37ff, 0xecc02cf0, 0xd08b2f83, 0x51a4d213, 0x6a811739,
			0x01727767, 0x5b7622b1, 0xc6ca8cca, 0x6b25494c, 0xca958919, 0x99a5515b,
			0xaf10e85b, 0xdec2c965, 0x77499264, 0x9d484ef7, 0xd6242f98, 0xe29393b6,
			0x685e86be, 0x1a2b9d6e, 0x8fa96dd7, 0xdec3e850, 0x186d5fe8, 0x40967d10,
			0x410e5956, 0xd0d6c4e7, 0xec9e2673, 0x3369eff8, 0x6e0a9e38, 0x62f67601,
			0x4122d3fc, 0xa7edf9a2, 0xbd1ee5e2, 0x16ed1a09, 0x4ee93acf, 0xe03312ef,
			0xe7aac0b6, 0xd61ec12a, 0x2f88804a, 0x66968d4f
		},
	},
#elif defined(CONFIG_RSA_KEY_SET_SPACECAST_PVT)
	/* key for PVT and later */
        { .n0inv = 195283641u,
	  .modulus = {
			0x00d0e477, 0xf2c79d07, 0x8e6d89f9, 0xf78e11ab, 0xc12d3843, 0x559d7cbc,
			0x6d2af8f0, 0x4acd7f44, 0x6a5efae4, 0x8bfd2668, 0xda9dcf5d, 0xdf4f7a24,
			0xc2e6a340, 0xc8f0d8a6, 0x5b805e11, 0x33d9501a, 0xe98ab7a5, 0xa5ea2d40,
			0x6abfb309, 0xefc13c55, 0xcf971694, 0xe8c226e8, 0xaee9ab3e, 0x03f7e300,
			0x6b02b6b5, 0x9c3d386e, 0x2396a7e9, 0x960c5616, 0x5e20ea20, 0xae450c6d,
			0x9bf99a51, 0x3f17fe6e, 0x811c13cf, 0x5fadcb0b, 0xe0dc6477, 0xf36c939f,
			0xcf37a23e, 0x664d2446, 0x586912e9, 0x48b405e7, 0x43c30911, 0x926170bd,
			0xb4620733, 0x082fafe8, 0x08bcbe39, 0xb7bb54ed, 0xbb24cbbc, 0x7b144d1b,
			0x7a1c3fea, 0x8df132d6, 0x0aeffcb3, 0x08bd3182, 0xf933d319, 0x319aecc3,
			0xdc366c76, 0xca5d0406, 0x9ec2f529, 0x81b62ee4, 0xaafaa8db, 0xd93fbd7d,
			0xec4462be, 0xb0d67c9e, 0x47baac54, 0xe7c44a68

		},
	  .rr = {
			0xed50ac6c, 0x253c9b12, 0xf4608904, 0x604dc138, 0xbd6c2924, 0xd139fb32,
			0x22e7bf33, 0xccb0c70a, 0x4037f129, 0xb04dd59d, 0x52de2a7f, 0x58686a91,
			0xb0a91c3c, 0x194eb38c, 0x33c496cd, 0xd72c196f, 0x8c486bbd, 0xcd55e8b7,
			0x02be129b, 0x59abe9f0, 0x75086721, 0x94ce8b3b, 0x2bf564e0, 0x373eadce,
			0xd83d0f68, 0x7b76112f, 0x3ea5a135, 0x91c32326, 0x4fab0c72, 0x29c0f214,
			0x53181873, 0x863c0247, 0x547d2e95, 0xc930f0ff, 0x446eda28, 0xf55ed8a4,
			0xed6f32ee, 0x065fb283, 0xdc7c32d0, 0x4a0c550e, 0xb5520fa8, 0xf84d2d81,
			0x85b7a578, 0x0c3f85f4, 0x01263508, 0xa348d505, 0xed6b3914, 0xdcae48fe,
			0xe067b525, 0x4e83a3d3, 0x0f9ff305, 0x5aa158dc, 0xea0e7b95, 0xe75d51a3,
			0x47cbb29e, 0x32ed0cf7, 0x075f3b6b, 0x9aba0a5c, 0x779a4ce7, 0x78cc852d,
			0xfd686bd6, 0x0e66ca26, 0xda579162, 0x5ddf1353
		},
	},
#else
        "unknown key set";
#endif
};
#else
/* Keys to verify the signature of the Linux kernel */
static const struct rsa_public_key public_keys[] = {
	/* Optimus / Sideswipe (same key as for Barebox) */
	{ .n0inv = 324625445u,
	  .modulus = {
			0xdd47e853, 0xcc40a072, 0xccc006f4, 0xc941d763, 0x8f53aa97, 0x26f1e34a,
			0xb476f79a, 0xf4aaaaaa, 0x676ae350, 0xd8b0d648, 0x18fc6347, 0x984bb7d3,
			0xa894b4a0, 0x51ea64c3, 0xeda9ea3c, 0xdcff20c7, 0x88afc95f, 0xa485fce7,
			0xd7010b97, 0x6e821430, 0xf836bbda, 0x004b61fe, 0xcddcc51a, 0x8e2f01f0,
			0x92647f40, 0xb60cc2e2, 0x570c6692, 0x40e51f0a, 0x34e9036f, 0x6b56ad55,
			0xe4d044e4, 0x2d32bd77, 0xfed334b5, 0x7505311f, 0x98f0fc46, 0xa557619e,
			0xb1158093, 0xf3a785c1, 0xf6e77d38, 0xe0e29c9b, 0x3aba8b7f, 0x212ee1f4,
			0x636549c6, 0x5bc9597b, 0xf2f35938, 0xf0ab94b7, 0x63519a73, 0x62dd79e6,
			0x3e0863f8, 0xf9d67dab, 0x2dbf66b7, 0x45bb69e6, 0xdc3836bb, 0xd253d7f4,
			0x975d5f43, 0x40eac67c, 0xd27139fa, 0xb2c73104, 0xced1c1f2, 0x7353f693,
			0xee23ed71, 0xc710fdf4, 0x330f3b81, 0xb4a49e97
		},
	  .rr = {
			0xc416ec5a, 0x8f35e335, 0x7860fdb6, 0xe3dd9018, 0xb021e1a8, 0x950621cb,
			0xb4d3c3e5, 0xe52c0288, 0x25366c47, 0xae481186, 0x8f4f42e4, 0x5203857d,
			0x60517eb4, 0xa49474c0, 0x952bdc12, 0xbdba6b4b, 0xac90fe1d, 0xbf7e29f9,
			0x41fb4116, 0xc2b9ca56, 0xf48e1cbd, 0xe7f43887, 0xebdf65f6, 0x694773f6,
			0x1efd33be, 0x4d7c0c9d, 0xd01b12f9, 0xde65788a, 0x93db7132, 0x8a8e23b1,
			0x0faab0b6, 0x7fd78d30, 0xe7e04e64, 0xdd0b2eab, 0xa93630d2, 0x07928be0,
			0x4ff324a5, 0x494b52da, 0xbed79044, 0x93d5c5b6, 0xdfb6b89b, 0x3c0ad0f5,
			0xdfaf19b6, 0x07d4e67f, 0xba2fc62a, 0x58697a86, 0x1c973e7f, 0x66a26c99,
			0x45a0ded5, 0x4f69d633, 0xbe326828, 0xd9d31655, 0x699b6a54, 0x2cb1ee8b,
			0x69dcbbc0, 0x9469b4fa, 0xf3b23e6b, 0x886ec86a, 0x2e9410a9, 0xa41074cf,
			0xa4c4ca4f, 0xd3ec5963, 0xf36f1929, 0x22bf4a0b
		}
	},

	/* SpaceCast */
#ifdef CONFIG_RSA_KEY_SET_DEFAULT
	{ .n0inv = 2823309225u,
	  .modulus = {
			0xfc50b367, 0x1fb911d9, 0x288cc6c6, 0x4914e135, 0x1f5b469a, 0x471274cd,
			0x21806adc, 0xa139ba92, 0x442f7796, 0x44183483, 0x2c68bad2, 0x6c5ca18a,
			0xbafd7213, 0x505fcebf, 0x0979e4b9, 0xa4582dec, 0x10e9e4de, 0x9ca9d12e,
			0xeea818ed, 0x9eff1262, 0xab58d203, 0x4095c3fd, 0x4e6e012a, 0x4c39b955,
			0xf64e8010, 0x4fc0a794, 0x967cbf92, 0xd39e2d03, 0xe0091a07, 0x140eda30,
			0xaf3ac27c, 0xbe82e9ff, 0x729b12ac, 0x98fae091, 0x6350a44a, 0xe10e0548,
			0x7fd91960, 0x4b903720, 0x3268a194, 0x6e880413, 0x768b5297, 0x24ce8d88,
			0x398356b3, 0x88ca3193, 0x458ecb86, 0xf25696a6, 0xd32c8972, 0x00e876d0,
			0x6c5716c3, 0x0efb7e2c, 0xa7d7fac6, 0xf6ef74ac, 0xe2bf77d7, 0x9be3b4e9,
			0x0f49d81d, 0x206060e3, 0x2f400bbe, 0x4e2bf784, 0x0a0b38eb, 0xdc4f319f,
			0xff1c7f2d, 0x80a57d93, 0xd38f9d6f, 0xc4c0a36b
		},
	  .rr = {
			0x1e81e6a8, 0x219f1c50, 0x2c0bb2f7, 0x51b3f5ff, 0x7d1af2de, 0x04c70430,
			0x67d2309f, 0xa6cae5a0, 0x80f18f2d, 0x1ecd7ef1, 0xbb509a3f, 0xbd37399b,
			0x486bd510, 0xf75f543d, 0xaeeea0a4, 0x797d3193, 0x6d88f643, 0x2b2d9034,
			0xaecae630, 0xcdfb16ee, 0x063a4148, 0x6ea42094, 0xca7682d4, 0xae7e5d02,
			0xe3010c28, 0x4f6dccfe, 0xb213e106, 0x113a5e32, 0x53dfe3f7, 0x86daefad,
			0x14d92242, 0xa19f3f5f, 0xb066f02f, 0x1d0f308d, 0x5602a6c1, 0x76ddf03e,
			0x978483d1, 0x0622cbe5, 0x0e5e2772, 0x88570f42, 0xeea9bbdc, 0xc12b4991,
			0xc4e63516, 0xf8e8707a, 0xecdee3bd, 0xc25c2079, 0x481f382f, 0x563c502b,
			0x1a341fd2, 0x8e2d10e4, 0xb2c60249, 0x3235d027, 0x32df7140, 0x9ffce524,
			0x1c8d6bf7, 0x003941ad, 0x74bf0d98, 0x18e77f88, 0xd8b38212, 0xc9b52ba2,
			0x137b5417, 0x114fddce, 0x096edf7c, 0x636ba76b
		},
	},
#elif defined(CONFIG_RSA_KEY_SET_SPACECAST_PVT)
	/* key for PVT and later */
	{ .n0inv = 3300387007u,
	  .modulus = {
			0x53c078c1, 0xb526325e, 0x35fc4264, 0xe521532c, 0xffffdae7, 0x13a83684,
			0x75a8dec3, 0x486c1103, 0x83413d5a, 0x5ff7284d, 0xe3edf2e3, 0xacdcfb69,
			0x47ff89fe, 0x2e4d6ce2, 0x8d408fb4, 0x18a1b7dc, 0x036b01d0, 0x96a9d88b,
			0x5a3f7f84, 0xa009556c, 0xdf7708a3, 0xdcb9a46c, 0x1098a771, 0x1d9a3950,
			0x5626f4fe, 0xa29a38b7, 0xef474395, 0x068c6b19, 0x0627915e, 0x6408316e,
			0xb8846e9f, 0x424d2353, 0x3459056c, 0xd4644766, 0xcbdd903e, 0x4ad11c03,
			0xcca5bae0, 0x39dc5fec, 0xd6a8c26f, 0x9db73de0, 0xbe8e7a02, 0x7867afb7,
			0xed12ea6c, 0x5389af08, 0x368253d6, 0xbdb7e3e7, 0x53a496c4, 0xe1887844,
			0xdf8c426b, 0x4eea6d02, 0x1a1a5a5a, 0x1c9cca85, 0x3ef25ef7, 0x7bda5b27,
			0xbb4a5a16, 0xdabee40a, 0x0dca1b83, 0xf520ca76, 0x7ac8dccd, 0x8499df2d,
			0x88d03426, 0x7284ac78, 0x61b59b11, 0xca19e021
		},
	  .rr = {
			0xa7cc85bf, 0xca308e0b, 0x983bd433, 0x91f76e99, 0x0163f6a6, 0x7cb98262,
			0xc252aedd, 0x01903495, 0xf3f4de55, 0x755ef407, 0xedc0f2e2, 0x7d298f3f,
			0x82513afd, 0x6c54a8a6, 0xb1039a7e, 0xbfd15569, 0x6a5a5ebc, 0xfbfd8266,
			0xda34f5c1, 0x03109310, 0xca683a32, 0xb884a224, 0x8b9e4529, 0x2ffed4e5,
			0x40599ce2, 0x73209335, 0x3b0799c8, 0x999a1bc7, 0x361413ce, 0xdbbfaf96,
			0xc26caaff, 0x6f829819, 0x49752b3d, 0x658f6b37, 0x388085c4, 0x5d1d7315,
			0xf20aa1e6, 0x4bb531f4, 0x51522975, 0x5bfddd05, 0x1a424abf, 0xb4ba7969,
			0xf13c8e57, 0x7eae1284, 0x510a90b6, 0xf64f59e4, 0x98634700, 0xcb0bf35e,
			0xcb739554, 0xaaae12fe, 0xa13dbc63, 0xac13a4c4, 0x2c264027, 0x1557ff1a,
			0x4d8e21ad, 0x91dcc873, 0xe870d738, 0x796a04cd, 0x9f81f3a2, 0xa8a6e714,
			0xfcc78693, 0xe8fdab84, 0x52a65ac1, 0x08ca4927
		},
	},
#else
        "unknown key set";
#endif
};
#endif

#ifdef CONFIG_COMCERTO_BOOTLOADER
/* Keys to verify the signature of a recovery image */
static const struct rsa_public_key recovery_keys[] = {
	/* Optimus / Sideswipe */
	{
	},

	/* SpaceCast */
	{ .n0inv = 2795114361u,
	  .modulus = {
			0xe17fd537, 0xa3c8cb00, 0x932ee36d, 0xc73c251a, 0xfab26fa4, 0x60c7ab00,
			0x1468dbfd, 0xcff11b21, 0xcff22a1d, 0x98057ec2, 0x052fc7ea, 0x8adc183c,
			0xcb273fca, 0xca676ea5, 0x32ec71b3, 0xf85c1297, 0xd1361b2f, 0x3b042ba7,
			0x6f1298a3, 0x3cbb1ec3, 0xf3f3236f, 0xc5911a46, 0x5763bb53, 0x8245bffc,
			0xca4164d8, 0xc1bf0062, 0x85471268, 0x68a0c01d, 0x3c296f90, 0xabd8d75d,
			0xe23924fa, 0xcc5662cf, 0x77a4d814, 0x113c5183, 0x31e559ba, 0xb6c08aa4,
			0x22a2bef9, 0x313739a1, 0xa8a57320, 0xab7241eb, 0x9e0c9dea, 0x01c255eb,
			0xe40b3989, 0x2f01f596, 0x8356d053, 0x78e23be2, 0x8f0d9f9f, 0x09022096,
			0x34a371a4, 0x0920ec0a, 0x5436f15d, 0x747537ec, 0xaf363081, 0xdbb6cc2e,
			0x229d8b82, 0x4b9b298c, 0x7bf3f4c9, 0xf2447fb3, 0xf95c482f, 0x4413e804,
			0x122bab97, 0x4f99ee1e, 0x148b2564, 0xad1c425d
		},
	  .rr = {
			0xb5e135c2, 0xe5d20b16, 0xd657f9f6, 0xe957d210, 0x9dfd33c9, 0x71bedfc7,
			0xeb570397, 0x356df96e, 0x30d89660, 0x90f4f10a, 0x88113810, 0x785cf0e1,
			0xb14858e8, 0xd7aeba86, 0xc7995ce2, 0x14872d4e, 0x1d87cf00, 0xa97490ff,
			0x8c4bdf64, 0x61dcbe9d, 0x989499eb, 0x4bd61725, 0x9625bf64, 0x288391af,
			0xbfaa0134, 0xf90b0d27, 0x951a1bd2, 0xa9f5468f, 0x76e41638, 0xefcb50f4,
			0xaa8eec52, 0x0421f434, 0xbf89794f, 0x08ed9a86, 0x9808d3e7, 0xc34c72f8,
			0xd25c759a, 0x10d0442c, 0x35e606fc, 0x977e88e8, 0x2c3b4f81, 0x973bcf11,
			0x7704760d, 0xbba4cbcd, 0xf0fe3a2b, 0x76813378, 0x90791b81, 0xad678e73,
			0x8620d2cc, 0x3fa28ae1, 0xdf9f91c3, 0xba2fa335, 0x4dffb2c0, 0x61c27230,
			0x98665fe1, 0x6378d57b, 0xcbb52ffd, 0xecda0f98, 0x69635ba9, 0xe9f2d4a1,
			0x8ae1b8d7, 0x079a8d27, 0x62294cff, 0x75017f67
		},
	},
};
#endif

static int get_key_id(int board_id) {
	if ((board_id < 0) || (board_id > MAX_BOARD_ID)) {
		printf("Invalid board ID: %d\n", board_id);
		return -1;
	}

	return board_id_to_key_index[board_id];
}

int rsa_get_public_key(int board_id, const struct rsa_public_key **key) {
	int index = get_key_id(board_id);
	if (index < 0) {
		return -1;
	}

	*key = &public_keys[index];

	return 0;
}

#ifdef CONFIG_COMCERTO_BOOTLOADER
int rsa_get_recovery_key(int board_id, const struct rsa_public_key **key) {
	int index = get_key_id(board_id);
	if (index < 0) {
		return -1;
	}

	*key = &recovery_keys[index];

	return 0;
}
#endif
