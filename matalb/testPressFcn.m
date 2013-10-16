function testPressFcn
    
   figure('KeyReleaseFcn',@cb)
   
      function cb(src,evnt)
      if ~isempty(evnt.Modifier)
         modifer=evnt.Modifier
         key=evnt.Key
            
         end
      
      end
      
end




