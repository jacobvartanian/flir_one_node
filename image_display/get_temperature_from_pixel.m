function [temperature] = get_temperature_from_pixel(pixel)
%GET_TEMPERATURE_FROM_PIXEL Gets the temperature (K) given a 16 bit pixel
%value
    r1 = 16528.178;
    b = 1427.5;
    f = 1.0;
    p0 = -1307.0;
    r2 = 0.012258549;
    reflected_temp_K = 293.15;  % 20 C
    emissivity = 0.95;
    
    % Correction factor
    pixel = double(pixel) * 4;
    
    % Determine radiance of reflected objects (emissivity < 1)
    radiance = r1 / (r2 * exp(b / reflected_temp_K) - f) - p0;
    
    object = (pixel - (1 - emissivity) * radiance) / emissivity;
    
    % Calculate temperature

    temperature = b / log(r1 / (r2 * (object + p0)) + f);
end

