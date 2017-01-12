--[[ psycle plugineditor (c) 2015, 2016 by psycledelics
File: maincanvas.lua
copyright 2015 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.  
]]

-- require('mobdebug').start()
-- local serpent = require("psycle.serpent")

local machine = require("psycle.machine")
local machines = require("psycle.machines")
local catcher = require("psycle.plugincatcher")
local command = require("psycle.command")

local cfg = require("psycle.config"):new("PatternVisual")
local systems = require("psycle.ui.systems")
local orderedtable = require("psycle.orderedtable")
local run = require("psycle.run")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local button = require("psycle.ui.button")
local boxspace = require("psycle.ui.boxspace")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local image = require("psycle.ui.image")
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local canvas = require("psycle.ui.canvas")
local splitter = require("psycle.ui.splitter")
local combobox = require("psycle.ui.combobox")

local filehelper = require("psycle.file")
local fileopen = require("psycle.ui.fileopen")
local filesave = require("psycle.ui.filesave")
local fileexplorer = require("fileexplorer")
local settingspage = require("settingspage")
local textpage = require("textpage")
local advancededit = require("advancededit")
local info = require("info")

local toolicon = require("psycle.ui.canvas.toolicon")
local toolbar = require("psycle.ui.canvas.toolbar")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local titlebaricons = require("psycle.ui.canvas.titlebaricons")
local separator = require("psycle.ui.canvas.separator")

local project = require("project")
local search = require("search")
local output = require("output")
local callstack = require("callstack")
local pluginselector = require("pluginselector")
local statusbar = require("statusbar")
local templateparser = require("templateparser")
local run = require("psycle.run")
local serpent = require("psycle.serpent")

local sci = require("scintilladef")

local maincanvas = canvas:new()

maincanvas.picdir = cfg:luapath().."\\psycle\\ui\\icons\\"

function maincanvas:new(machine)
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c.machine_ = machine  
  c:init()
  return c
end

function maincanvas:init()
  self:link(require("psycle.ui.standarduisheet"))
  self:link(require("skin"))
  self.machines = machines:new()  
  self:invalidatedirect()
  self:initstyleclasses(self.machine_.settingsmanager:setting())
  self:setupfiledialogs()  
  self:createcreateeditplugin()  
  self:inittoolbar()
  self:initinfo()  
  self:createsearch()
  self:createoutputs()
  splitter:new(self, splitter.HORZ)  
  self.fileexplorer = self:createfileexplorer()   
  self.pluginselector.fileexplorer = self.fileexplorer
  splitter:new(self, splitter.VERT)
  self:createpagegroup()
  systems:new():updatewindows()
  self.cursormovement = 1  
end

function maincanvas:initinfo()
  self.info = info:new(self)
                  :setalign(item.ALTOP)
                  :setautosize(false, false)
                  :setposition(rect:new(point:new(), dimension:new(0, 160)))  
  self.info.cmd:connect(maincanvas.oncmdinfo, self)
  self.statusbar:setinfo(self.info)
end

function maincanvas:oncmdinfo(cmd)  
  if (self.pages:activepage()) then    
    if cmd == "sethelplevel0" then
      self.info.helplevel = 0                 
    elseif cmd == "sethelplevel1" then
      self.info.helplevel = 1
    elseif cmd == "sethelplevel2" then
      self.info.helplevel = 2
    elseif cmd == "sethelplevel3" then
      self.info.helplevel = 3
      if not self.info:visible() then
        self.info:show()      
        self:updatealign();    
      end
    elseif cmd == "saveresume" then
      self:savepage()
    elseif cmd == "savedone" then
      self:savepage()
      self.pages:removepage(self.pages:activepage())
    elseif cmd == "saveexit" then
      self:savepage()
      dtesself.machine_:exit()
    elseif cmd == "abandonfile" then
      self.pages:removepage(self.pages:activepage())
    else    
      self.pages:activepage():oncmd(cmd)
    end    
    self.statusbar.status.prompttext:settext("")
    if cmd == "findtext" then
       self:displaysearch() 
    else
      self:onstatuslineescape()
    end
  end  
end

function maincanvas:createcreateeditplugin()  
  self.pluginselector 
      = pluginselector:new(self)
                      :hide()
                      :setalign(item.ALTOP)
                      :setautosize(false, false)
                      :setposition(rect:new(point:new(), dimension:new(0, 200)))
  self.pluginselector.doopen:connect(maincanvas.onopenplugin, self)
  self.pluginselector.docreate:connect(maincanvas.oncreateplugin, self)
  self.pluginselector.docreatemodule:connect(maincanvas.oncreatemodule, self)
end

function maincanvas:onopenplugin(pluginpath, pluginname, info)  
  self:openplugin(pluginpath, pluginname, info)
  psycle.proxy.project = project:new():setplugininfo(info)
  self:fillinstancecombobox()  
  self.machine_:settitle(pluginname)
end

function maincanvas:oncreateplugin(outputs, machinename)  
  for _, output in pairs(outputs) do  
    local fname = string.sub(output.path, 1, -5)    
    self:openplugin(machinename.."\\"..fname, machinename)
  end             
  
  local catcher = catcher:new():rescannew()
  --local infos = catcher:infos()        
  --for i=1, #infos do       
   -- if infos[i]:type() == machine.MACH_LUA and
   --    infos[i]:name():lower() == pluginname:lower() then      
    --  psycle.proxy.project = project:new():setplugininfo(infos[i])
    --  break;     
  --  end
  --end      
  self.machineselector:settext(machinename)  
  self.pluginselector:hide()
  self:updatealign()
  self:setfocus()
end

function maincanvas:oncreatemodule(outputs)    
  for _, output in pairs(outputs) do    
    self:openfromfile(output.realpath)    
  end  
  self.pluginselector:hide()
  self.fileexplorer:update()
  self:setfocus()  
  self:updatealign()  
  self:invalidate()
end

function maincanvas:createsearch()
  local setting = self.machine_.settingsmanager:setting()
  self.search
      = search:new(self, setting)
              :setposition(rect:new(point:new(), dimension:new(200, 200)))
              :hide()
              :setalign(item.ALBOTTOM)
  self.search:applysetting(setting)           
  self.search.dosearch:connect(maincanvas.onsearch, self)
  self.search.doreplace:connect(maincanvas.onreplace, self)
  self.search.dohide:connect(maincanvas.onsearchhide, self)
  self.searchbeginpos = -1
  self.searchrestarted = false
end

function maincanvas:createoutputs()
  self.outputs 
       = tabgroup:new(self)
                 :setposition(rect:new(point:new(), dimension:new(0, 180)))
                 :setalign(item.ALBOTTOM)
  self.output = output:new()  
  self.outputs:addpage(self.output, "Output")    
  self.callstack = self:createcallstack()  
  self.outputs:addpage(self.callstack, "Call stack")  
end

function maincanvas:setindicator(page)
   page:f(sci.SCI_INDICSETSTYLE, 8, sci.INDIC_PLAIN)
   page:f(sci.SCI_INDICSETSTYLE, 9, sci.INDIC_SQUIGGLE)
   page:f(sci.SCI_INDICSETSTYLE, 10, sci.INDIC_TT)
   page:f(sci.SCI_INDICSETSTYLE, 11, sci.INDIC_DIAGONAL)
   page:f(sci.SCI_INDICSETSTYLE, 12, sci.INDIC_STRIKE)
   page:f(sci.SCI_INDICSETSTYLE, 13, sci.INDIC_BOX)
   page:f(sci.SCI_INDICSETSTYLE, 14, sci.INDIC_ROUNDBOX)
   page:f(sci.SCI_SETINDICATORCURRENT, 14, 0)
   page:f(sci.SCI_POSITIONFROMLINE, 14, 0)
   page:f(sci.SCI_INDICATORFILLRANGE, 2, 5)
end

function maincanvas:createcallstack()
  local callstack = callstack:new()
  callstack.change:connect(maincanvas.openinfo, self)  
  return callstack
end

function maincanvas:createpagegroup()
  self.pages = tabgroup:new(self)
                       :setalign(item.ALCLIENT)             
  self.pages.dopageclose:connect(maincanvas.onclosepage, self)  
end

function maincanvas:createfileexplorer()
  local fileexplorer = fileexplorer:new(self, self.machine_.settingsmanager:setting().fileexplorer)
                                   :setposition(rect:new(point:new(), dimension:new(200, 0)))
                                   :setalign(item.ALLEFT)
  fileexplorer:setpath(cfg:luapath())  
  fileexplorer.click:connect(maincanvas.onpluginexplorerclick, self)  
  --pluginexplorer.onremove:connect(maincanvas.onpluginexplorernoderemove, self)
  return fileexplorer
end

function maincanvas:setupfiledialogs()
  self.fileopen = fileopen:new()
  local that = self
  function self.fileopen:onok(fname)    
    that:openfromfile(fname)
  end
  self.filesaveas = filesave:new() 
end

function maincanvas:setoutputtext(text)
  self.output:addtext(text)  
end

function maincanvas:setplugindir(plugininfo)  
  local path = plugininfo:dllname()
  path = path:sub(1, -5).."\\"
  self.fileexplorer:setpath(path)
  self.machineselector:settext(plugininfo:name())  
end

function maincanvas:dopageexist(fname)
  local found = nil
  local items = self.pages.children:items()    
  for i=1, #items do
    local page = items[i]
    if page:filename() == fname then
      found = page
      break
    end
  end
  return found
end

function maincanvas:createpage()
  local page = textpage:new(nil, self.machine_.settingsmanager:setting())
                       :setmargin(boxspace:new(2, 0, 0, 0))
  systems:new():updatewindow(textpage.windowtype, page)                         
  return page
end

function maincanvas:onkeydown(ev)  
  if ev:ctrlkey() then
    local done = false
    if self.cursormovement == 1 and self.pages:activepage() then
      if ev:keycode() == 0x41 then
        self.pages:activepage():wordleft()
        done = true      
        ev:stoppropagation()
      elseif ev:keycode() == 0x46 then
        self.pages:activepage():wordright()
        done = true      
        ev:stoppropagation()
      elseif ev:keycode() == 0x53 then                
        self.pages:activepage():charleft()
        done = true      
        ev:stoppropagation()
      elseif ev:keycode() == 0x44 then
        self.pages:activepage():charright()
        done = true      
        ev:stoppropagation()
      elseif ev:keycode() == 0x45 then
        self.pages:activepage():lineup()
        done = true      
        ev:stoppropagation()             
      elseif ev:keycode() == 0x58 then
        self.pages:activepage():linedown()
        done = true      
        ev:stoppropagation()      
      end
    end
    if not done then      
      if ev:keycode() == 0x4A or ev:keycode() == 0x4B or ev:keycode() == 0x51 then
        self.statusbar:setcontrolkey(ev:keycode())        
        self.info:jump(ev:keycode())
        self.statusbar.status.control:setfocus()
      elseif ev:keycode() == 0x47 then
        self.statusbar.status.line:setfocus()
        ev:stoppropagation()
        self.info:deactivate()
      elseif ev:keycode() == 78 then
        self:createnewpage()   
        ev:stoppropagation()
      elseif ev:keycode() == 79 then
        self.fileopen:setfolder(self.fileexplorer:path())      
        self.fileopen:show()          
        ev:stoppropagation()
      elseif ev:keycode() == 87 then         
        ev:stoppropagation()
        self:closeandremovepage()
      elseif ev:keycode() == 70 then
        self:displaysearch() 
        ev:stoppropagation()
      elseif ev:keycode() == 83 then             
        self:savepage()
        ev:stoppropagation()
      elseif ev:keycode() == ev.F5  then
        self:playplugin()
        ev:stoppropagation()
      end
    end  
  end  
end

function maincanvas:closeandremovepage()  
  if (self.pages:activepage()) then
    local e = {}
    e.page = self.pages:activepage()
    self:onclosepage(e);        
    self.pages:removepage(e.page)   
  end     
end

function maincanvas:openfromfile(fname, line)
  if not line  then line = 0 end
  local page = self:dopageexist(fname)  
  if page ~= nil then
    page:reload()
    if self.pages:activepage() ~= page then   
      self.pages:setactivepage(page)
    end
  else    
    page = self:createpage()
    page:loadfile(fname)    
    local sep = "\\"  
    local dir = fname:match("(.*"..sep..")")    
    --self:addfiletowatcher(dir)  
    local name = fname:match("([^\\]+)$")           
    self.pages:addpage(page, name)         
  end  
  page:gotoline(line - 1)    
end

function maincanvas:setcallstack(trace)
  for i=1, #trace do    
    self.callstack:addline(trace[i])
  end  
end

function maincanvas:createnewpage()
  local page = self:createpage()
  self.pages:addpage(page, page:createdefaultname())  
end

function maincanvas:savepage()  
  local page = self.pages:activepage()
  if page then
    local fname = ""     
    if page:hasfile() then
    fname = page:filename()    
    local sep = "\\"  
      local dir = fname:match("(.*"..sep..")")    
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pages:setlabel(page, fname)
    else    
    self.filesaveas:setfolder(self.fileexplorer:path())
    if self.filesaveas:show() then      
      fname = self.filesaveas:filename()          
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pages:setlabel(page, fname)
    end
    end 
  end
end

function maincanvas:playplugin()  
  local pluginindex = psycle.proxy.project:pluginindex()  
  if pluginindex ~= -1 then
     machine = machine:new(pluginindex)
     if machine then
       --self:savepage()
       machine:reload()        
     self.playicon:seton(true)     
     end
  else
    --self:savepage()    
    if  psycle.proxy.project:plugininfo() then
      local fname = psycle.proxy.project:plugininfo():dllname():match("([^\\]+)$"):sub(1, -5)    
      machine = machine:new(fname)      
      local pluginindex = self.machines:insert(machine)    
      psycle.proxy.project:setpluginindex(pluginindex)    
      self:fillinstancecombobox()
      self:setpluginindex(pluginindex)
    self.playicon:seton(true)   
    end
  end  
end

function maincanvas:inittoolbar()  
  self.tg = group:new(self)
                 :setautosize(false, true)
                 :setalign(item.ALTOP)
                 :setmargin(boxspace:new(0, 0, 5, 0))  
  self.tgsmall = group:new(self)
                      :setautosize(false, true)
                      :setalign(item.ALTOP)                 
  self.windowtoolbar = titlebaricons:new(self.tg)
  local that = self
  function self.windowtoolbar.settingsicon:onclick()
    local settingspage = settingspage:new(nil, that.machine_)                                        
    if settingspage then 
      settingspage:setmargin(boxspace:new(2, 0, 0, 0))
      settingspage.doapply:connect(maincanvas.onapplysetting, that)      
      that.pages:addpage(settingspage, "Settings")
    end    
  end
  self:initmachineselector(self.tg)
  self:initfiletoolbar():setalign(item.ALLEFT)
  separator:new(self.tg):setalign(item.ALLEFT):setmargin(boxspace:new(10))
  self:initplaytoolbar():setalign(item.ALLEFT)
  self:initstatus()  
end

function maincanvas:createreloadicon()
  local reloadicon = toolicon:new(self.tg, maincanvas.picdir.."reload.png", 0xFFFFFF)
  self:formattoolicon(reloadicon)
  reloadicon:setalign(item.ALRIGHT)                
  local that = self
  function reloadicon:onclick()
    that.machine_:reload()
  end  
end

function maincanvas:initmachineselector(parent)
  self.machineselector = 
      advancededit:new(parent)
                  :setalign(item.ALLEFT)
                  :setposition(rect:new(point:new(), dimension:new(200, 40)))
  local that = self  
  function self.machineselector:onmousedown(ev)    
    that.pluginselector:show()
    that:updatealign() 
    advancededit.onmousedown(self, ev)  
  end  
  function self.machineselector:onkillfocus(ev)    
    that:updatealign()  
    advancededit.onkillfocus(self, ev)  
  end  
  function self.machineselector:onkeydown(ev)    
    advancededit.onkeydown(self, ev)
    if ev:keycode() == ev.RETURN then
      that:onmachineselectorreturnkey(self:text())
      that.pluginselector:hide()
    end 
    if ev:keycode() == ev.ESCAPE then      
      that.pluginselector:hide()
      that:updatealign()
    end
  end
  function self.machineselector:onkeyup(ev)
    if self.selectorhasfocus then
      that.pluginselector:searchandfocusmachine(self:text())
      that:updatealign()    
    end
    advancededit.onkeyup(self, ev)  
  end  
end

function maincanvas:onmachineselectorreturnkey(text)
  local node = self.pluginselector:search(text)
  if node then
    self.pluginselector:open(node)
  end  
end

function maincanvas:initstatus()    
  local setting = self.machine_.settingsmanager:setting()
  self.statusbar = statusbar:new(self.tgsmall, setting)
  self.statusbar.gotopage:connect(maincanvas.ongotopage, self)
  self.statusbar.escape:connect(maincanvas.onstatuslineescape, self)
end

function maincanvas:ongotopage(edit)
  if self.pages:activepage() then   
     self.pages:activepage():gotoline(tonumber(edit:text()) - 1)
  end
end

function maincanvas:onstatuslineescape(edit)
  if self.pages:activepage() then   
     self.pages:activepage():setfocus()
  end
end

function maincanvas:initwindowtoolbar()    
  local icon = toolicon:new(self.tg, maincanvas.picdir .. "arrow_out.png", 0xFFFFFF)
  icon.is_toggle = true  
  local windowinimg = image:new()
                            :load(maincanvas.picdir .. "arrow_in.png")
                            :settransparent(0xFFFFFF)                                 
  icon:settoggleimage(windowinimg)
  self:formattoolicon(icon)
  icon:setalign(item.ALRIGHT)
  local that = self
  function icon:onclick()
    self:resethover()    
    that.machine_:toggleviewport()      
  end
  return icon        
end

function maincanvas:fillinstancecombobox()
  local items = {"new instance"}
  self.cbxtopluginindex = {-1}
  if (psycle.proxy.project:plugininfo()) then
    for machineindex= 0, 255 do
      local machine = machine:new(machineindex);       
      if machine and machine:type() == machine.MACH_LUA and machine:pluginname() == psycle.proxy.project:plugininfo():name() then
        items[#items + 1] = machine:pluginname() .. "[" .. machineindex .. "]"
        self.cbxtopluginindex[#self.cbxtopluginindex + 1] = machineindex
      end
    end     
  end
  self.cbx:setitems(items)
  self.cbx:setitemindex(1)
  return self
end  

function maincanvas:setpluginindex(pluginindex)
  local cbxindex = 1  
  for i = 1, #self.cbxtopluginindex do    
    if self.cbxtopluginindex[i] == pluginindex then       
       cbxindex = i
       break    
    end
  end  
  self.cbx:setitemindex(cbxindex)  
end

function maincanvas:createinstanceselect(parent)  
  self.cbx = combobox:new(parent)
                     :setautosize(false, false)
                     :setposition(rect:new(point:new(0, 0), dimension:new(150, 0)))
           :setalign(item.ALLEFT)
           :setmargin(boxspace:new(10))
  local that = self
  function self.cbx:onselect()    
    local pluginindex = that.cbxtopluginindex[self:itemindex()]
    if pluginindex then
      psycle.proxy.project:setpluginindex(pluginindex)       
    else
      psycle.proxy.project:setpluginindex(-1)       
    end
  end
  self.cbx:setitems({"new instance"})
  self.cbx:setitemindex(1)
end

function maincanvas:formattoolicon(icon)    
  icon:setautosize(false, false)
  icon:setverticalalignment(toolicon.ALCENTER)
  icon:setjustify(toolicon.CENTERJUSTIFY)
  icon:setposition(rect:new(point:new(), dimension:new(40, 0)))
  return self
end

function maincanvas:initfiletoolbar()  
  local t = toolbar:new(self.tg)
                   :setautosize(true, false)
  local icons = {  
    inew = toolicon:new(t, maincanvas.picdir.."new.png", 0xFFFFFF),                      
    iopen = toolicon:new(t, maincanvas.picdir.."open.png", 0xFFFFFF),                        
    isave = toolicon:new(t, maincanvas.picdir.."save.png", 0xFFFFFF),
    iundo = toolicon:new(t, maincanvas.picdir.."undo.png", 0xFFFFFF),
    iredo = toolicon:new(t, maincanvas.picdir.."redo.png", 0xFFFFFF)
  }
  for _, icon in pairs(icons) do    
    self:formattoolicon(icon)
  end
  
  self.newpagecommand = command:new()
  local that = self
  function self.newpagecommand:execute()
    local page = that:createpage()
    that.pages:addpage(page, page:createdefaultname())  
  end
  icons.inew:setcommand(self.newpagecommand)
  
  --icons.inew.click:connect(maincanvas.createnewpage, self)
  icons.iopen.click:connect(maincanvas.onopenfile, self)
  icons.isave.click:connect(maincanvas.savepage, self)
  icons.iredo.click:connect(maincanvas.redo, self)
  icons.iundo.click:connect(maincanvas.undo, self)  
  return t
end

function maincanvas:onopenfile(icon)
  icon:resethover()
  self.fileopen:setfolder(self.fileexplorer:path())  
  self.fileopen:show()
end

function maincanvas:initplaytoolbar()
  local t = toolbar:new(self.tg)
                   :setautosize(true, false)  
  self.playicon = toolicon:new(t, maincanvas.picdir.."poweroff.png", 0xFFFFFF)                          
  self:formattoolicon(self.playicon)
  self:formattoolicon(self.playicon)
  self.playicon.is_toggle = true
  local on = image:new():load(maincanvas.picdir.."poweron.png")
  on:settransparent(0xFFFFFF)
  self.playicon:settoggleimage(on)    
  local that = self
  function self.playicon:onclick()
    that:playplugin()
  end
  self:createinstanceselect(t)
  return t
end

function maincanvas:openinfo(info)
  local isfile = false
  if info.source:len() > 1 then
     local firstchar = info.source:sub(1, 1)     
     if firstchar == "@" then
       local fname = info.source:sub(2) 
       self:openfromfile(fname, info.line)
       isfile = true
     end
  end
  return isfile
end

function maincanvas:onsearch(searchtext, dir, case, wholeword, regexp)
  local page = self.pages:activepage()
  if page then 
    page:onsearch(searchtext, dir, case, wholeword, regexp)
  end
end

function maincanvas:onreplace()
  local page = self.pages:activepage()
  if page then 
    if page:hasselection() then     
      page:replacesel(self.search.replacefield:text())      
    end                 
    self:onsearch(self.search.edit:text(), 
                  self.search.DOWN,
                  self.search.casesensitive:check(),
                  self.search.wholeword:check(),
                  self.search.useregexp:check())
  end
end

function maincanvas:displaysearch(ev)
  self.search:show():onfocus()
  self:updatealign()  
end

function findlast(haystack, needle)
  local i=haystack:match(".*"..needle.."()")
  if i==nil then return else return i end
end

function maincanvas:openplugin(pluginpath, pluginname, plugininfo)  
  self:preventdraw()
  self:closealltabs()      
  self.fileexplorer:sethomepath(cfg:luapath().."\\"..pluginpath:sub(1, pluginpath:find("\\") - 1))  
  self:openfromfile(cfg:luapath().."\\"..pluginpath..".lua")
  self:setfocus()
  self.machineselector:settext(pluginname)
  self:updatealign()
  self:enabledraw()  
  self:invalidate()
end

function maincanvas:closealltabs()    
  self.pages:removeall()
  textpage.pagecounter = 1
end

function maincanvas:onpluginexplorerclick(ev)
   if ev.filename ~= "" and ev.path then       
     self:openfromfile(ev.path.."\\"..ev.filename, 0)
   end
end

function maincanvas:onpluginexplorernoderemove(node)  
  node:parent():remove(node)
  self.pluginexplorer:updatetree()
  local items = self.pages.children:items()
  for i = 1, #items do        
    if node.path..node.filename == items[i]:filename() then
      self.pages:removepage(items[i])
      filehelper.remove(node.path..node.filename)
      break;
    end
  end  
end

function maincanvas:onidle()
  local activepage = self.pages:activepage()
  if activepage and activepage.status then  
    self.statusbar:updatestatus(activepage:status())    
  end  
  self:updatepowericon()  
  self.info:onidle(self)  
end

function maincanvas:updatepowericon()
  if self.cbxtopluginindex ~= nil then    
    local isselectedmachinemute = self.machines:muted(self.cbxtopluginindex[self.cbx:itemindex()])  
    if self.playicon:on() == isselectedmachinemute then      
    self.playicon:toggle()
    end
  end  
end

function maincanvas:onsearchhide()
  self.search:hide()
  self:updatealign()
  if self.pages:activepage() then
    self.pages:activepage():setfocus()
  end
end

function maincanvas:onclosepage(ev)  
  if ev.page:modified() and psycle.confirm("Do you want to save changes to "..ev.page:filename().."?") then
    --self:savepage()
  end
end

function maincanvas:undo()  
  if self.pages:activepage() then
    self.pages:activepage():undo()
  end
end

function maincanvas:redo()
  if self.pages:activepage() then
    self.pages:activepage():redo()
  end
end

function maincanvas:mergeproperties(source1, source2)
  local result = orderedtable.new()
  for name, property in source1:opairs() do    
    result[name] = property:clone()    
  end  
  for name, property in source2:opairs() do
    result[name] = property:clone()  
  end
  return result
end

function maincanvas:onapplysetting(setting)    
  self:initstyleclasses(setting)
  systems:new():updatewindows()
end

function maincanvas:initstyleclasses(setting)  
  local systems = systems:new()
  --systems:setstyleclass(systems.windowtypes.EDIT, setting.general.children.ui.children.edit.properties)
  --systems:setstyleclass(systems.windowtypes.LISTVIEW, setting.general.children.ui.children.listview.properties)
  --systems:setstyleclass(systems.windowtypes.TREEVIEW, setting.general.children.ui.children.treeview.properties)
  --systems:setstyleclass(toolicon.windowtype, setting.general.children.ui.children.toolicon.properties)
  systems:setstyleclass(advancededit.windowtype, setting.general.children.ui.children.advancededit.properties)
  systems:setstyleclass(statusbar.windowtype, setting.statusbar.properties)
  --systems:setstyleclass(tabgroup.windowtype, setting.general.children.ui.children.tabgroup.properties)
  systems:setstyleclass(output.windowtype, setting.output.properties)  
  systems:setstyleclass(textpage.windowtype, 
     self:mergeproperties(setting.textpage.properties, self:mergeproperties(
  setting.lualexer.properties, setting.general.properties)))
     --systems:setstyleclass(info.windowtype, setting.general.properties)
  self:addornament(ornamentfactory:createfill(setting.general.properties.backgroundcolor:value()))
end

return maincanvas
