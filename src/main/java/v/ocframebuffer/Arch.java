/*
  Copyright, V.

  This file is part of OC Framebuffer.

  OC Framebuffer is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OC Framebuffer is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ME Storage Processor.  If not, see <http://www.gnu.org/licenses/>.
*/

package v.ocframebuffer;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import li.cil.oc.api.internal.TextBuffer;
import li.cil.oc.api.machine.Architecture;
import li.cil.oc.api.machine.ExecutionResult;
import li.cil.oc.api.machine.Machine;
import li.cil.oc.api.machine.Signal;

@Architecture.Name("OCFramebuffer")
public class Arch implements Architecture {
  private final Machine machine;
  private TextBuffer screen;
  private shmem mem;
  private long handle;

  public Arch(Machine machine) {
    this.machine = machine;
    this.screen = null;
    this.mem = new shmem();
  }
 
  @Override
  public boolean isInitialized() {
    return true;
  }
 
  @Override
	public boolean recomputeMemory(Iterable<ItemStack> components) {
    return true;
  }
 
  @Override
  public boolean initialize() {
//     final ItemStack stack = li.cil.oc.api.Items.get("screen1").createItemStack(1);
//     final TextBuffer buffer = (TextBuffer) li.cil.oc.api.Driver.driverFor(stack).createEnvironment(stack, this);

  	Map<String, String> components = this.machine.components();
		
    for (String key: components.keySet()) {
      String value = components.get(key);
      switch (value) {
        case "screen": {
          screen = (TextBuffer)machine.node().network().node(key).host();
          break;
        }
      }
    }

    if (screen != null) {
  		screen.setMaximumResolution(80, 25);
		  screen.setResolution(80, 25);
		  screen.setViewport(80, 25);
		  screen.setPowerState(true);
		  screen.fill(0, 0, 80, 25, ' ');
      screen.set(0, 0, "Starting external application...", false);
		}
		
    handle = mem.get_handle();

    return true;
  }
 
  @Override
  public void close() {
    // FIXME: Kill external process, close shared mem.
  }
 
  @Override
  public ExecutionResult runThreaded(boolean isSynchronizedReturn) {
    try {
      final Signal signal;

      if (isSynchronizedReturn) {
        signal = null;
      } else {
        signal = machine.popSignal();
      }

      // FIXME: Push keyboard/mouse event signals back into shared memory.

      return new ExecutionResult.SynchronizedCall();
    }
    catch (Throwable t) {
      return new ExecutionResult.Error(t.toString());
    }
  }
 
  @Override
  public void runSynchronized() {
    ByteBuffer buffer = mem.get_next_frame(handle);
    
    if (screen != null && buffer != null) {
      byte[] dst = new byte[80];

      for (int row = 0; row < 25; row++) {
        buffer.get(dst, 0, 80);
        screen.set(0, row, new String(dst, StandardCharsets.ISO_8859_1), false);
      }
    }
  }
 
  @Override
  public void onConnect() {
  }
 
  @Override
  public void load(NBTTagCompound nbt) {
  }
 
  @Override
  public void save(NBTTagCompound nbt) {
  }

  @Override
	public void onSignal() {
    // FIXME: Not in the API. Do we use this then for keyboard / mouse events?
  }
}
