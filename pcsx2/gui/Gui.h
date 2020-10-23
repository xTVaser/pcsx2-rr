    #pragma once

    #include <iostream>
    
    
    /*// ------------------------------------------------------------------------
	struct FolderOptions
	{
			bool
				UseDefaultBios,
				UseDefaultSnapshots,
				UseDefaultSavestates,
				UseDefaultMemoryCards,
				UseDefaultLogs,
				UseDefaultLangs,
				UseDefaultCheats,
				UseDefaultCheatsWS;

		std::string
			Bios,
			Snapshots,
			Savestates,
			MemoryCards,
			Langs,
			Logs,
			Cheats,
			CheatsWS;

		fs::path RunIso;		// last used location for Iso loading.
		fs::path RunELF;		// last used location for ELF loading.
		fs::path RunDisc;		// last used location for Disc loading.

		FolderOptions();
		nlohmann::json LoadSave();
		void ApplyDefaults();

		void Set( FoldersEnum_t folderidx, const std::string& src, bool useDefault );

		const std::string& operator[]( FoldersEnum_t folderidx ) const;
		std::string& operator[]( FoldersEnum_t folderidx );
		bool IsDefault( FoldersEnum_t folderidx ) const;
	};


    	// ------------------------------------------------------------------------
	// The GS window receives much love from the land of Options and Settings.
	//
	struct GSWindowOptions
	{
		// Closes the GS/Video port on escape (good for fullscreen activity)
		bool		CloseOnEsc;

		bool		DefaultToFullscreen;
		bool		AlwaysHideMouse;
		bool		DisableResizeBorders;
		bool		DisableScreenSaver;

		AspectRatioType AspectRatio;
		FMVAspectRatioSwitchType FMVAspectRatioSwitch;
		float	Zoom;
		float	StretchY;
		float	OffsetX;
		float	OffsetY;


		int			WindowSize[2]; // X, Y
		int			WindowPos[2]; // X, Y
		bool		IsMaximized;
		bool		IsFullscreen;
		bool		EnableVsyncWindowFlag;

		bool		IsToggleFullscreenOnDoubleClick;

		GSWindowOptions();

		nlohmann::json LoadSave();
		void SanityCheck();
	};


	struct UiTemplateOptions 
    {
		UiTemplateOptions();
			
		nlohmann::json LoadSave();

		std::string LimiterUnlimited;
		std::string LimiterTurbo;
		std::string LimiterSlowmo;
		std::string LimiterNormal;
		std::string OutputFrame;
		std::string OutputField;
		std::string OutputProgressive;
		std::string OutputInterlaced;
		std::string Paused;
		std::string TitleTemplate;
#ifndef DISABLE_RECORDING
		std::string RecordingTemplate;
#endif
	};*/ // LEGACY CODE 

