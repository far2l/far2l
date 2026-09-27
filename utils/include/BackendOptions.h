struct BackendOptions {
	bool UseModernLook;     	// unicode glyphs for check boxes / radio buttons
	bool UseRoundedBorders; 	// wx only: rounded corners for borders
	bool UseSingleBordersOnly;	// wx only: replace double borders to single
	bool UseNoBorders;			// wx only: replace double borders to single
	bool UseEmbossAsBold;		// wx only: repplace bold glyphs to emboss effect
	bool UseSoftenBevels;		// wx only: makes boxes less bright for black and white case
	bool Use3D;                 // wx only: 3d buttons

	bool UseFlyGirl;            // wx only: show/hide fly girl watermark 
	wchar_t FlyGirls[2048];
	int FlyGirlAnchor;          // 0 - left-top, top, right-top, right, right-bottom, bottom, left-bottom, left. center
	int FlyGirlPaddingX;        // 40
	int FlyGirlPaddingY;        // 40
	int FlyGirlMaxTransparency; // 40
	int FlyGirlLessTnanToTransparent; // RGB
	int FlyGirlMoreThanToTransparent; // RGB
	int FlyGirlMaxWidth;   // -1 upscale, 0 unchanged, 200px
	int FlyGirlMaxHeight;  // -1 upscale, 0,unchanged, 200px
};
