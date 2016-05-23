function [nln,nbn] = select(fem, dim)
%SELECT returns the number of local DOFS of a finite element type
%
%   [NLN,NBN] = SELECT(FEM, DIM) given a type of finite elements FEM (string)
%   and the spatial dimension DIM, returns the number of local dofs on the 
%   reference element NLN and  the number of local boundary dofs NBN

if nargin < 2 || isempty(dim)
    dim = 2;
end

switch dim
    
    case 2
        
        switch fem
            
            case 'P1'
                nln = 3;
                nbn = 2;
                
            case 'P2'
                
                nln = 6;
                nbn = 3;
                
            case 'B1'
                
                nln = 4;
                nbn = 2;     
                
            otherwise
                
                error([fem, 'FE space not available in 2D']);
        end

        
    case 3
        
        switch fem
            case 'P1'
                nln = 4;
                nbn = 3;
            case 'P2'
                nln = 10;
                nbn = 6;
            case 'B1'
                nln = 5;
                nbn = 3;
            otherwise
                error([fem, 'FE space not available in 3D']);
        end
        
end

return
