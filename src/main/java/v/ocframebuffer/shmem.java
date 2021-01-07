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

public class shmem {
  static {
    try {
//      System.loadLibrary("shmem");
//    	System.load(System.getProperty("java.io.tmpdir") + "shmem.dll");
    	System.load("D:\\Desktop\\OCFramebuffer\\shmem.dll");
    } catch (Exception e) {
      System.err.println("Library failed to load.");
      System.exit(1);
    }
  }

  public native long get_handle();
  public native ByteBuffer get_next_frame(long handle);
}
