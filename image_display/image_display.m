close all;
clear;

KELVIN_OFFSET = 273.15;

pixel_offset_error = [-0.015, 0.015];
ir_scaling = 4;

ir_image_sub = rossubscriber('/flirone/images/ir_16b');
rgb_image_sub = rossubscriber('/flirone/images/rgb_jpg');
pause(2);

figure(1);

hold on;

centre_text_h = text(250,500,'');
centre_text_h.FontSize = 16;
centre_text_h.Color = 'g';

min_text_h = text(0,500,'');
min_text_h.FontSize = 16;
min_text_h.Color = 'b';

max_text_h = text(500,500,'');
max_text_h.FontSize = 16;
max_text_h.Color = 'r';

figure(2);

while(1)
    rgb_image_data = receive(rgb_image_sub);
    rgb_image = readImage(rgb_image_data);
    ir_image_data = receive(ir_image_sub);
    ir_image = readImage(ir_image_data);
    
    % Set scaling parameters
    ir_to_rgb_scale = size(rgb_image, [1, 2]) ./ size(ir_image);
    rgb_scaling = ir_scaling / ir_to_rgb_scale(1);
    
    % remove last 2 rows of IR image as they are broken
    ir_image = ir_image(1:end-2,:);
    ir_to_rgb_scale_adjusted = size(rgb_image, [1, 2]) ./ size(ir_image);
    
    % Get max value and location
    [max_in_each_row, max_index_in_each_row] = max(ir_image);
    [max_ir, max_row] = max(max_in_each_row);
    max_col = max_index_in_each_row(max_row);
    max_index = [max_col, max_row];
    max_temperature = get_temperature_from_pixel(max_ir);
    
    % Get min value and location
    [min_in_each_row, min_index_in_each_row] = min(ir_image);
    [min_ir, min_row] = min(min_in_each_row);
    min_col = min_index_in_each_row(min_row);
    min_index = [min_col, min_row];
    min_temperature = get_temperature_from_pixel(min_ir);
    
    % Get centre temperature
    centre_index = size(ir_image) ./ 2;
    centre_temperature = get_temperature_from_pixel(ir_image(centre_index(1), centre_index(2)));
    
    % Scale image from min to max, 8 bit
    pixel_range = max_ir - min_ir;
    scaled_ir = (double(ir_image) - double(min_ir)) ./ double(pixel_range);
    
    % Adjust image sizes to match

    scaled_ir = imresize(scaled_ir, ir_scaling);
    scaled_rgb = imresize(rgb_image, rgb_scaling);
    scaled_min_index_ir = ir_scaling * min_index;
    scaled_max_index_ir = ir_scaling * max_index;
    scaled_centre_index_ir = ir_scaling * centre_index;
    ir_to_rgb_scaling = size(rgb_image, [1, 2]) / size(ir_image);
    scaled_min_index_rgb = rgb_scaling .* ir_to_rgb_scale_adjusted .* min_index + pixel_offset_error .* size(rgb_image, [1, 2]);
    scaled_max_index_rgb = rgb_scaling .* ir_to_rgb_scale_adjusted .* max_index + pixel_offset_error .* size(rgb_image, [1, 2]);
    scaled_centre_index_rgb = rgb_scaling .* ir_to_rgb_scale_adjusted .* centre_index + pixel_offset_error .* size(rgb_image, [1, 2]);
    
    % Display IR
    figure(1);
    delete(findobj(gca, 'type', 'line'));
    imshow(scaled_ir);
    hold on;
    plot(scaled_centre_index_ir(2), scaled_centre_index_ir(1), 'g+', 'MarkerSize', 50);
    plot(scaled_max_index_ir(2), scaled_max_index_ir(1), 'r+', 'MarkerSize', 50);
    plot(scaled_min_index_ir(2), scaled_min_index_ir(1), 'b+', 'MarkerSize', 50);
    centre_text_str = [num2str(centre_temperature - KELVIN_OFFSET), char(176), 'C'];
    min_text_str = [num2str(min_temperature - KELVIN_OFFSET), char(176), 'C'];
    max_text_str = [num2str(max_temperature - KELVIN_OFFSET), char(176), 'C'];
    centre_text_h.String = centre_text_str;
    min_text_h.String = min_text_str;
    max_text_h.String = max_text_str;
    
    % Display RGB
    figure(2);
    delete(findobj(gca, 'type', 'line'));
    imshow(scaled_rgb);
    hold on;
    plot(scaled_centre_index_rgb(2), scaled_centre_index_rgb(1), 'g+', 'MarkerSize', 50);
    plot(scaled_max_index_rgb(2), scaled_max_index_rgb(1), 'r+', 'MarkerSize', 50);
    plot(scaled_min_index_rgb(2), scaled_min_index_rgb(1), 'b+', 'MarkerSize', 50);
end