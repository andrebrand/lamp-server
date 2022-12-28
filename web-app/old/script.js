$(document).ready( () =>  {
    let applyedColor = 'white';
    let selectedColor = 'white';
    let mode = 'fade'; // or static



    $('.inset-color').click((e)=>{
        if(mode === 'static'){
            t = $(e.target);
            const newColor = t.data('name');
            if(selectedColor !== newColor){
                //cange color in preview
                selectedColor = newColor;
                const currentColor =  $('.selected-color').data('color');
                $('.selected-color').data('color', newColor);
                $('.selected-color').addClass(newColor);
                $('.selected-color').removeClass(currentColor);
                //show buttons
                $('#cancelColor').addClass('show');
                $('#applyColor').addClass('show');
            }
        }
    });
    $('#cancelColor').click(()=> {
        if(mode === 'static'){
            selectedColor = applyedColor;
            //cange color in preview
            const newColor = selectedColor;
            const currentColor =  $('.selected-color').data('color');
            $('.selected-color').data('color', newColor);
            $('.selected-color').addClass(newColor);
            $('.selected-color').removeClass(currentColor);
            // hide buttons
            $('#cancelColor').removeClass('show');
            $('#applyColor').removeClass('show');
        }
    });
    $('#applyColor').click(()=> {
        if(mode === 'static'){
            applyedColor = selectedColor;
            // hide buttons
            $('#cancelColor').removeClass('show');
            $('#applyColor').removeClass('show');
            const color$ = $('.inset-color.'+ applyedColor);
            const r = color$.data('r');
            const g = color$.data('g');
            const b = color$.data('b');
            console.log('r', r, 'g', g, 'b', b);
            // ToDo send rgb

            $.get('/change?r='+ r + '&g=' + g + '&b=' + b, function(data, status){
                console.log('data', data);
                console.log('status', status);
            });
        }
    });

    $('#modeFade').click(()=>{
        if(!$('#modeFade').hasClass('active')){
            $('#static-colors').addClass('hide');
            mode = 'fade';
            $.get('/fade?fade=1', function(data, status){
                console.log('data', data);
                console.log('status', status);
            });

            $('#modeStatic').removeClass('active');
            $('#modeFade').addClass('active');
        }
    });
    $('#modeStatic').click(()=>{
        if(!$('#modeStatic').hasClass('active')){
            $('#static-colors').removeClass('hide');
            mode = 'static';
            $.get('/fade?fade=0', function(data, status){
                console.log('data', data);
                console.log('status', status)
            });
            $('#modeFade').removeClass('active');
            $('#modeStatic').addClass('active');
            //show buttons
            //$('#cancelColor').addClass('show');
            $('#applyColor').addClass('show');
        }
    });



});
