﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger
{
	public class BreakpointManager
	{
		private static List<Breakpoint> _breakpoints = new List<Breakpoint>();

		public static event EventHandler BreakpointsChanged;
		public static List<Breakpoint> Breakpoints { get { return _breakpoints; } }

		public static void RefreshBreakpoints(Breakpoint bp = null)
		{
			if(BreakpointsChanged != null) {
				BreakpointsChanged(bp, null);
			}

			SetBreakpoints();
		}

		public static void EditBreakpoint(Breakpoint bp)
		{
			new frmBreakpoint(bp).ShowDialog();
			RefreshBreakpoints(bp);
		}

		public static void RemoveBreakpoint(Breakpoint bp)
		{
			Breakpoints.Remove(bp);
			RefreshBreakpoints(bp);
		}

		public static void AddBreakpoint(Breakpoint bp)
		{
			Breakpoints.Add(bp);
			RefreshBreakpoints(bp);
		}

		public static Breakpoint GetMatchingBreakpoint(int address)
		{
			return Breakpoints.FirstOrDefault<Breakpoint>((bp) => { return bp.Address == address; });
		}

		public static void ToggleBreakpoint(int address, bool toggleEnabled)
		{
			if(address >= 0) {
				Breakpoint breakpoint = BreakpointManager.GetMatchingBreakpoint(address);
				if(breakpoint != null) {
					if(toggleEnabled) {
						breakpoint.SetEnabled(!breakpoint.Enabled);
					} else {
						BreakpointManager.RemoveBreakpoint(breakpoint);
					}
				} else {
					breakpoint = new Breakpoint() {
						BreakOnExec = true,
						Address = (UInt32)address,
						IsAbsoluteAddress = false,
						Enabled = true
					};
					BreakpointManager.AddBreakpoint(breakpoint);
				}
			}
		}

		public static void SetBreakpoints()
		{
			List<InteropBreakpoint> breakpoints = new List<InteropBreakpoint>();
			foreach(Breakpoint bp in BreakpointManager.Breakpoints) {
				if(bp.Enabled) {
					breakpoints.Add(bp.ToInteropBreakpoint());
				}
			}
			InteropEmu.DebugSetBreakpoints(breakpoints.ToArray(), (UInt32)breakpoints.Count);
		}
	}
}
