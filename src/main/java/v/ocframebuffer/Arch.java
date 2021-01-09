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
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
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
    if (screen == null || !mem.get_next_frame(handle)) {
      return;
    }

    // Native returns arrays of direct allocated ByteBuffers, so with the right view it should be zero copy.
    // Take that Java! I win! ;-). Proper framebuffers (VGA Card/VGA Gpu) will get a 32bit full color bytebuffer.
    //
    // ps. Another way to fix this is passing preallocated double arrays to native and let native fill that.
    // But that would result in copying in native (will be faster for TextBuffer as it would replace the for below).
    //
    // FIXME: Are rawSetText, rawSetForeground and rawSetbackground broken in OC?

    byte[] content_row = new byte[80];
    int[] color_row = new int[80];
    int[][] foreground_color = new int[25][80];
    int[][] background_color = new int[25][80];

    ByteBuffer[] content_buffer = mem.get_content_buffer(handle);
    ByteBuffer[] foreground_color_buffer = mem.get_foreground_color_buffer(handle);
    ByteBuffer[] background_color_buffer = mem.get_background_color_buffer(handle);

    for (int row = 0; row < 25; row++) {
      content_buffer[row].get(content_row);
      screen.set(0, row, new String(content_row, Charset.forName("cp437")), false);
//      screen.set(0, row, new String(content_row, StandardCharsets.ISO_8859_1), false);
    }

/*
// Colorum est rumpitur.

//      foreground_color_buffer[row].asIntBuffer().get(color_row);
//      foreground_color[row] = color_row;

//      background_color_buffer[row].asIntBuffer().get(color_row);
//      background_color[row] = color_row;
    int[][] foreground_color_test = new int[3][3];
    int[][] background_color_test = new int[3][3];

    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        foreground_color_test[row][col] = (row * 0x400000) | 0x00ff00 | (col * 0x000020);
        background_color_test[row][col] = (row * 0x000020) | (col * 0x400000);
      }
    }
    
    screen.rawSetForeground(0, 0, foreground_color_test);
    screen.rawSetBackground(0, 0, background_color_test);

    screen.data.color[3][3] = 0x050a;
*/
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
