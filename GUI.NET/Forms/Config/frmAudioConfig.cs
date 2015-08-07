﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmAudioConfig : BaseConfigForm
	{
		public frmAudioConfig()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.AudioInfo;

			AddBinding("EnableAudio", chkEnableAudio);
			AddBinding("MasterVolume", trkMaster);
			AddBinding("Square1Volume", trkSquare1Vol);
			AddBinding("Square2Volume", trkSquare2Vol);
			AddBinding("TriangleVolume", trkTriangleVol);
			AddBinding("NoiseVolume", trkNoiseVol);
			AddBinding("DmcVolume", trkDmcVol);
			AddBinding("AudioLatency", nudLatency);
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			base.OnFormClosed(e);
			AudioInfo.ApplyConfig();
		}

		private void AudioConfig_ValueChanged(object sender, EventArgs e)
		{
			if(!this.Updating) {
				UpdateObject();
				AudioInfo.ApplyConfig();
			}
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			AudioInfo config = Entity as AudioInfo;
			config.EnableAudio = true;
			config.AudioLatency = 100;
			config.MasterVolume = 50;
			config.Square2Volume = 50;
			config.Square1Volume = 50;
			config.TriangleVolume = 50;
			config.NoiseVolume = 50;
			config.DmcVolume = 50;
			UpdateUI();
			AudioInfo.ApplyConfig();
		}
	}
}