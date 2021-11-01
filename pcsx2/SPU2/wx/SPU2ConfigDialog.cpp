/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"

#include "SPU2ConfigDialog.h"

#include "PortAudioSettingsDialog.h"

#include "SPU2/SndOutModule.h"

#include "../Config.h"
#include "../wx/Config.h"

MixerTab::MixerTab(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	auto* top_box = new wxBoxSizer(wxVERTICAL);

	// Mixing Settings
	top_box->Add(new wxStaticText(this, wxID_ANY, "Interpolation"), wxSizerFlags().Centre());

	wxArrayString interpolation_entries;
	interpolation_entries.Add("Nearest (Fastest / worst quality)");
	interpolation_entries.Add("Linear (Simple / okay sound)");
	interpolation_entries.Add("Cubic (Fake highs / okay sound)");
	interpolation_entries.Add("Hermite (Better highs / okay sound)");
	interpolation_entries.Add("Catmull-Rom (PS2-like / good sound)");
	interpolation_entries.Add("Gaussian (PS2-like / great sound)");

	m_inter_select = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, interpolation_entries);

	// Latency Slider
	const int min_latency = SynchMode == 0 ? LATENCY_MIN_TIMESTRETCH : LATENCY_MIN;

	m_latency_box = new wxStaticBoxSizer(wxVERTICAL, this, "Latency");
	m_latency_slider = new wxSlider(this, wxID_ANY, SndOutLatencyMS, min_latency, LATENCY_MAX, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
	m_latency_box->Add(m_latency_slider, wxSizerFlags().Expand());

	// Volume Slider
	m_volume_box = new wxStaticBoxSizer(wxVERTICAL, this, "Volume");
	m_volume_slider = new wxSlider(this, wxID_ANY, FinalVolume * 100, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
	m_volume_box->Add(m_volume_slider, wxSizerFlags().Expand());

	m_audio_box = new wxBoxSizer(wxVERTICAL);
	m_audio_box->Add(new wxStaticText(this, wxID_ANY, "Audio Expansion Mode"), wxSizerFlags().Centre());

	wxArrayString audio_entries;
	audio_entries.Add("Stereo (None, Default)");
	audio_entries.Add("Quadrafonic");
	audio_entries.Add("Surround 5.1");
	audio_entries.Add("Surround 7.1");
	m_audio_select = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, audio_entries);
	m_audio_box->Add(m_audio_select, wxSizerFlags().Expand());

	top_box->Add(m_inter_select, wxSizerFlags().Centre());
	top_box->Add(m_latency_box, wxSizerFlags().Expand());
	top_box->Add(m_volume_box, wxSizerFlags().Expand());
	top_box->Add(m_audio_box, wxSizerFlags().Expand());

	SetSizerAndFit(top_box);
}

void MixerTab::Load()
{
	m_inter_select->SetSelection(Interpolation);

	m_audio_select->SetSelection(numSpeakers);

	m_volume_slider->SetValue(FinalVolume * 100);
	m_latency_slider->SetValue(SndOutLatencyMS);
}

void MixerTab::Save()
{
	Interpolation = m_inter_select->GetSelection();

	numSpeakers = m_audio_select->GetSelection();

	FinalVolume = m_volume_slider->GetValue() / 100.0;
	SndOutLatencyMS = m_latency_slider->GetValue();
}

void MixerTab::Update()
{
}

void MixerTab::CallUpdate(wxCommandEvent& /*event*/)
{
	Update();
}

SyncTab::SyncTab(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	auto* top_box = new wxBoxSizer(wxVERTICAL);

	top_box->Add(new wxStaticText(this, wxID_ANY, "Synchronization"), wxSizerFlags().Centre());

	wxArrayString sync_entries;
	sync_entries.Add("TimeStretch (Recommended)");
	sync_entries.Add("Async Mix (Breaks some games!)");
	sync_entries.Add("None (Audio can skip.)");
	m_sync_select = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sync_entries);

	auto* adv_box = new wxStaticBoxSizer(wxVERTICAL, this, "Advanced");

	auto* babble_label = new wxStaticText(this, wxID_ANY,
		"For fine-tuning time stretching.\n"
		"Larger is better for slowdown, && smaller for speedup (60+ fps).\n"
		"All options in microseconds.",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	babble_label->Wrap(300);

	adv_box->Add(babble_label, wxSizerFlags().Centre());

	auto* soundtouch_grid = new wxFlexGridSizer(2, 10, 50);

	seq_spin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, SoundtouchCfg::SequenceLen_Min, SoundtouchCfg::SequenceLen_Max, SoundtouchCfg::SequenceLenMS);
	auto* seq_label = new wxStaticText(this, wxID_ANY, "Sequence Length");

	seek_spin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, SoundtouchCfg::SeekWindow_Min, SoundtouchCfg::SeekWindow_Max, SoundtouchCfg::SeekWindowMS);
	auto* seek_label = new wxStaticText(this, wxID_ANY, "Seek Window Size");

	overlap_spin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, SoundtouchCfg::Overlap_Min, SoundtouchCfg::Overlap_Max, SoundtouchCfg::OverlapMS);
	auto* overlap_label = new wxStaticText(this, wxID_ANY, "Overlap");

	soundtouch_grid->Add(seq_label, wxSizerFlags().Border(wxALL, 5));
	soundtouch_grid->Add(seq_spin, wxSizerFlags().Expand().Right());
	soundtouch_grid->Add(seek_label, wxSizerFlags().Border(wxALL, 5));
	soundtouch_grid->Add(seek_spin, wxSizerFlags().Expand().Right());
	soundtouch_grid->Add(overlap_label, wxSizerFlags().Border(wxALL, 5));
	soundtouch_grid->Add(overlap_spin, wxSizerFlags().Expand().Right());

	adv_box->Add(soundtouch_grid);

	reset_button = new wxButton(this, wxID_ANY, "Reset To Defaults");
	adv_box->Add(reset_button, wxSizerFlags().Centre().Border(wxALL, 5));

	top_box->Add(m_sync_select, wxSizerFlags().Centre());
	top_box->Add(adv_box, wxSizerFlags().Expand().Centre());
	SetSizerAndFit(top_box);
	Bind(wxEVT_BUTTON, &SyncTab::OnButtonClicked, this);
	Bind(wxEVT_CHOICE, &SyncTab::CallUpdate, this);
}

void SyncTab::Load()
{
	m_sync_select->SetSelection(SynchMode);

	SoundtouchCfg::ReadSettings();
	seq_spin->SetValue(SoundtouchCfg::SequenceLenMS);
	seek_spin->SetValue(SoundtouchCfg::SeekWindowMS);
	overlap_spin->SetValue(SoundtouchCfg::OverlapMS);
}

void SyncTab::Save()
{
	SynchMode = m_sync_select->GetSelection();

	SoundtouchCfg::SequenceLenMS = seq_spin->GetValue();
	SoundtouchCfg::SeekWindowMS = seek_spin->GetValue();
	SoundtouchCfg::OverlapMS = overlap_spin->GetValue();
	SoundtouchCfg::WriteSettings();
}

void SyncTab::Update()
{
	seq_spin->Enable(m_sync_select->GetCurrentSelection() == 0);
	seek_spin->Enable(m_sync_select->GetCurrentSelection() == 0);
	overlap_spin->Enable(m_sync_select->GetCurrentSelection() == 0);
}

void SyncTab::CallUpdate(wxCommandEvent& /*event*/)
{
	Update();
}

void SyncTab::OnButtonClicked(wxCommandEvent& event)
{
	seq_spin->SetValue(30);
	seek_spin->SetValue(20);
	overlap_spin->SetValue(10);
}

DebugTab::DebugTab(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	auto* top_box = new wxBoxSizer(wxVERTICAL);

	debug_check = new wxCheckBox(this, wxID_ANY, "Enable Debug Options");
	show_check = new wxCheckBox(this, wxID_ANY, "Show in console");
	top_box->Add(debug_check, wxSizerFlags().Expand());
	top_box->Add(show_check);

	m_console_box = new wxStaticBoxSizer(wxVERTICAL, this, "Events");
	auto* console_grid = new wxFlexGridSizer(2, 0, 0);

	key_check = new wxCheckBox(this, wxID_ANY, "Key On/Off");
	voice_check = new wxCheckBox(this, wxID_ANY, "Voice Stop");
	dma_check = new wxCheckBox(this, wxID_ANY, "DMA Operations");
	autodma_check = new wxCheckBox(this, wxID_ANY, "AutoDMA Operations");
	buffer_check = new wxCheckBox(this, wxID_ANY, "Buffer Over/Underruns");
	adpcm_check = new wxCheckBox(this, wxID_ANY, "ADPCM Cache");

	console_grid->Add(key_check);
	console_grid->Add(voice_check);
	console_grid->Add(dma_check);
	console_grid->Add(autodma_check);
	console_grid->Add(buffer_check);
	console_grid->Add(adpcm_check);
	m_console_box->Add(console_grid);

	m_log_only_box = new wxStaticBoxSizer(wxVERTICAL, this, "Log Only");
	auto* log_grid = new wxFlexGridSizer(2, 0, 0);

	dma_actions_check = new wxCheckBox(this, wxID_ANY, "Register/DMA Actions");
	dma_writes_check = new wxCheckBox(this, wxID_ANY, "DMA Writes");
	auto_output_check = new wxCheckBox(this, wxID_ANY, "Audio Output");

	log_grid->Add(dma_actions_check);
	log_grid->Add(dma_writes_check);
	log_grid->Add(auto_output_check);
	m_log_only_box->Add(log_grid);

	dump_box = new wxStaticBoxSizer(wxVERTICAL, this, "Dump on Close");
	auto* dump_grid = new wxFlexGridSizer(2, 0, 0);

	core_voice_check = new wxCheckBox(this, wxID_ANY, "Core && Voice Stats");
	memory_check = new wxCheckBox(this, wxID_ANY, "Memory Contents");
	register_check = new wxCheckBox(this, wxID_ANY, "Register Data");
	dump_grid->Add(core_voice_check);
	dump_grid->Add(memory_check);
	dump_grid->Add(register_check);
	dump_box->Add(dump_grid);

	top_box->Add(m_console_box, wxSizerFlags().Expand());
	top_box->Add(m_log_only_box, wxSizerFlags().Expand());
	top_box->Add(dump_box, wxSizerFlags().Expand());

	SetSizerAndFit(top_box);
	Bind(wxEVT_CHECKBOX, &DebugTab::CallUpdate, this);
}

void DebugTab::Load()
{
	debug_check->SetValue(DebugEnabled);

	show_check->SetValue(_MsgToConsole);
	key_check->SetValue(_MsgKeyOnOff);
	voice_check->SetValue(_MsgVoiceOff);
	dma_check->SetValue(_MsgDMA);
	autodma_check->SetValue(_MsgAutoDMA);
	buffer_check->SetValue(_MsgOverruns);
	adpcm_check->SetValue(_MsgCache);

	dma_actions_check->SetValue(_AccessLog);
	dma_writes_check->SetValue(_DMALog);
	auto_output_check->SetValue(_WaveLog);

	core_voice_check->SetValue(_CoresDump);
	memory_check->SetValue(_MemDump);
	register_check->SetValue(_RegDump);
	Update();
}

void DebugTab::Save()
{
	DebugEnabled = debug_check->GetValue();

	_MsgToConsole = show_check->GetValue();
	_MsgKeyOnOff = key_check->GetValue();
	_MsgVoiceOff = voice_check->GetValue();
	_MsgDMA = dma_check->GetValue();
	_MsgAutoDMA = autodma_check->GetValue();
	_MsgOverruns = buffer_check->GetValue();
	_MsgCache = adpcm_check->GetValue();

	_AccessLog = dma_actions_check->GetValue();
	_DMALog = dma_writes_check->GetValue();
	_WaveLog = auto_output_check->GetValue();

	_CoresDump = core_voice_check->GetValue();
	_MemDump = memory_check->GetValue();
	_RegDump = register_check->GetValue();
}

void DebugTab::Update()
{
	if (debug_check->GetValue())
	{
		show_check->Enable();

		key_check->Enable();
		voice_check->Enable();
		dma_check->Enable();
		autodma_check->Enable();
		buffer_check->Enable();
		adpcm_check->Enable();

		dma_actions_check->Enable();
		dma_writes_check->Enable();
		auto_output_check->Enable();

		core_voice_check->Enable();
		memory_check->Enable();
		register_check->Enable();
	}
	else
	{
		show_check->Disable();

		key_check->Disable();
		voice_check->Disable();
		dma_check->Disable();
		autodma_check->Disable();
		buffer_check->Disable();
		adpcm_check->Disable();

		dma_actions_check->Disable();
		dma_writes_check->Disable();
		auto_output_check->Disable();

		core_voice_check->Disable();
		memory_check->Disable();
		register_check->Disable();
	}
}

void DebugTab::CallUpdate(wxCommandEvent& /*event*/)
{
	Update();
}

void SPU2ConfigDialog::onModuleConfig(wxCommandEvent& event)
{
	auto mod = m_module_select->GetSelection();

	wxDialog* dialog = nullptr;

	if (mods[mod] != nullptr)
	{
		auto ident = mods[mod]->GetIdent();

		if (ident == "portaudio")
		{
			dialog = new PortAudioSettingsDialog(this);
			dialog->ShowModal();
			dialog->Destroy();
		}

		// TODO:
		// - SDLAudio - linux only
	}
}

bool SPU2ConfigDialog::moduleHasPreferences()
{
	auto mod = m_module_select->GetSelection();
	if (mods[mod] != nullptr)
	{
		auto ident = mods[mod]->GetIdent();

		return ident == "portaudio" || ident == "SDLAudio";
	}
	return false;
}

void SPU2ConfigDialog::onModuleSelect(wxCommandEvent& event)
{
	m_backend_config->Enable(moduleHasPreferences());
}

SPU2ConfigDialog::SPU2ConfigDialog()
	: wxDialog(nullptr, wxID_ANY, "Audio Settings", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxBoxSizer* sizerWrapper = new wxBoxSizer(wxVERTICAL);
	m_top_box = new wxBoxSizer(wxVERTICAL);
	sizerWrapper->Add(m_top_box, 0, wxALL, 10);
	auto* module_box = new wxBoxSizer(wxVERTICAL);

	// Module
	module_box->Add(new wxStaticText(this, wxID_ANY, "Module"), wxSizerFlags().Centre());

	wxArrayString module_entries;

	for (auto i = 0; mods[i] != nullptr; i++)
	{
		module_entries.Add(mods[i]->GetLongName());
	}

	auto* module_contents = new wxBoxSizer(wxHORIZONTAL);
	module_box->Add(module_contents);
	m_module_select = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, module_entries);
	m_backend_config = new wxButton(this, wxID_PREFERENCES);

	module_contents->Add(m_module_select, wxSizerFlags().Centre());
	module_contents->Add(m_backend_config);

	m_top_box->Add(module_box, wxSizerFlags().Centre().Border(wxALL, 5));

	auto* book = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	m_mixer_panel = new MixerTab(book);
	m_sync_panel = new SyncTab(book);
	m_debug_panel = new DebugTab(book);

	book->AddPage(m_mixer_panel, "Mixing", true);
	book->AddPage(m_sync_panel, "Sync");
	book->AddPage(m_debug_panel, "Debug");

	m_top_box->Add(book, wxSizerFlags().Centre());
	m_top_box->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Right());

	// Bindings
	m_backend_config->Bind(wxEVT_BUTTON, &SPU2ConfigDialog::onModuleConfig, this);
	m_module_select->Bind(wxEVT_CHOICE, &SPU2ConfigDialog::onModuleSelect, this);
	Bind(wxEVT_CHOICE, &SPU2ConfigDialog::CallReconfigure, this);
	Bind(wxEVT_CHECKBOX, &SPU2ConfigDialog::CallReconfigure, this);

	// Init Dialog
	m_backend_config->Enable(moduleHasPreferences());

	SetSizer(sizerWrapper);
	sizerWrapper->Fit(this);
	Layout();
}

void SPU2ConfigDialog::Reconfigure()
{
	Layout();
}

void SPU2ConfigDialog::CallReconfigure(wxCommandEvent& event)
{
	Reconfigure();
}

void SPU2ConfigDialog::Load()
{
	m_module_select->SetSelection(OutputModule);
	m_mixer_panel->Load();
	m_sync_panel->Load();
	m_debug_panel->Load();

	Reconfigure();
}

void SPU2ConfigDialog::Save()
{
	OutputModule = m_module_select->GetSelection();

	m_mixer_panel->Save();
	m_sync_panel->Save();
	m_debug_panel->Save();
}

// Main
void SPU2ConfigDialog::Display()
{
	Load();
	if (ShowModal() == wxID_OK)
		Save();
}
