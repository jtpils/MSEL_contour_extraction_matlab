function [ Gx ] = Gaussian_x( sigma, dx, dy)
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here
half_size = ceil(sigma*4+max(abs(dx),abs(dy)));
[X,Y] = meshgrid(-half_size:half_size,-half_size:half_size);
Gx = -(X-dx).*exp(-((X-dx).^2+(Y-dy).^2)/(2*sigma^2))/(2*pi*sigma^4); 
end

