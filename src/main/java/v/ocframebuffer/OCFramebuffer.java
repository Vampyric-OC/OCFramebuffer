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

import net.minecraftforge.fml.common.Mod;
import net.minecraftforge.fml.common.Mod.EventHandler;
import net.minecraftforge.fml.common.event.FMLInitializationEvent;
import net.minecraftforge.fml.common.event.FMLPreInitializationEvent;
import org.apache.logging.log4j.Logger;

@Mod(modid = OCFramebuffer.MODID, name = OCFramebuffer.NAME, version = OCFramebuffer.VERSION)
public class OCFramebuffer {
  public static final String MODID = "ocframebuffer";
  public static final String NAME = "OC Framebuffer";
  public static final String VERSION = "0.1";

  private static Logger logger;
  private static shmem mem;
  private static long handle;

  @EventHandler
  public void preInit(FMLPreInitializationEvent event) {
    logger = event.getModLog();
  }

  @EventHandler
  public void init(FMLInitializationEvent event) {
    li.cil.oc.api.Machine.add(Arch.class);
    logger.info("Architecture added.");
  }
}
