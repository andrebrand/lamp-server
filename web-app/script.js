class LampConfig{
    doc;
    colorPickerDisabled = false;
    pickedColor = '';
    defaultColor = 'red';
    activeTab = 'home-tab';
    elements = {};

    colors = {
        'red': { r: 1, g: 0, b: 0},
        'pink': { r: 1, g: 0, b: 1},
        'yellow': { r: 1, g: 1, b: 0},
        'green': { r: 0, g: 1, b: 0},
        'cyan': { r: 0, g: 1, b: 1},
        'blue': { r: 0, g: 0, b: 1},
        'white': { r: 1, g: 1, b: 1},
        'black': { r: 0, g: 0, b: 0}
    };

    constructor(document){
        this.doc = document;
        this.elements.ssid = this.doc.getElementById('inputSSID');
        this.elements.wifipw = this.doc.getElementById('inputPW');
        this.elements.wifiValueHelp = this.doc.getElementById('wifiValueHelp');
        this.elements.modal = this.doc.getElementById('modal');
        this.elements.colorPreview = this.doc.getElementById('colorPreview');
        this.elements.backdrop = this.doc.getElementById('backdrop');
        this.elements.networkForm = this.doc.getElementById('networkForm');
        this.start();
    }

    start(){
        this.addModalListener();

        //Set initial color mode
        this.setInitalConfig();
    
        //Add listener to tabs
        this.addEventListenerToList(this.doc.getElementsByClassName('nav-link'), (e)=>{
            e.preventDefault();
            this.toggleTabs(e.target);
        });
    
        //Add listener to radio buttons
        this.addEventListenerToList(this.doc.getElementsByName('gridRadios'),  (e) => {
            this.toggleRadioButtons(e.target.value == 'fade');
        });
    
        //Add listener to color boxes
        this.addEventListenerToList(this.doc.getElementsByClassName('colorButton'),  (e) => {
            this.changeColor(e.target.id);
        });
    
        //add listener to form
        this.elements.networkForm.onsubmit = (e)=>{
            e.preventDefault();
            this.submitWifi();
        };

        this.addEventListenerToList([this.elements.wifiValueHelp],  (e) => {
            this.openWifiValueHelp();
        });

    }

    
    sendHttpRequest(url, method, data, callback, timeout){
        var xhr = new XMLHttpRequest();
        xhr.open(method, url, true);
        xhr.timeout = 5000;
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.ontimeout = (e) => {
            timeout();
        };
        xhr.onload = callback;
        
        if(!!data){
            xhr.send(JSON.stringify(data));
        }else{
            xhr.send();
        }
    }
    
    toggleColorPicker(){
        this.doc.getElementById('color-wrapper').classList.toggle('disabled');
    }
    
    addEventListenerToList(array, callback){
        let prev = null;
        for(let i = 0; i < array.length; i++) {
            array[i].onclick = callback;
        }
    }
    
    clearColorPreview(){
        this.elements.colorPreview.classList.remove('red', 'pink', 'yellow', 'green', 'cyan', 'blue', 'white', 'black');
    }
    
    changeColor(color){
        if(!this.colorPickerDisabled){
            const colorValues = this.colors[color];
            if(color !== this.pickedColor){
                this.clearColorPreview();
                this.sendHttpRequest('change?r='+ colorValues.r + '&g=' + colorValues.g + '&b=' + colorValues.b, 'GET', null, (e)=>{
                    this.elements.colorPreview.classList.add(color);
                    this.pickedColor = color;
                });
            }
        }
    }
    
    changeColorMode(mode){
        this.sendHttpRequest('/fade?fade='+ mode, 'GET', null, (e)=>{});
    }
    
    isValidSSID(value){
        return value.length >= 2 && value.length <= 32;
    }
    
    isValidWifiPW(value){
        return value.length >= 8 && value.length <= 70;
    }
    
    
    showModal(title, body, icon){
        this.doc.getElementById('modalLabel').innerHTML = title;
        this.doc.getElementById('modalContent').innerHTML = body;
        this.elements.modal.classList.add('show');
        this.elements.backdrop.classList.add('show');
        switch (icon) {
            case 'e':
                this.doc.getElementById('error-icon').classList.add('show');  
                break;
            case 'i':
                this.doc.getElementById('info-icon').classList.add('show');
                break;
        }
    }
    
    hideModal(){
        this.elements.modal.classList.remove('show');
        this.elements.backdrop.classList.remove('show');
        const icons = this.doc.getElementsByClassName('modal-icon');
        for (const icon of icons) {
            icon.classList.remove('show');
        }
    }
    
    addModalListener(){
        const elements = [];
        elements.push(this.elements.modal);
        elements.push(this.doc.getElementById('closeModalX'));
        elements.push(this.doc.getElementById('closeModal'));
        this.addEventListenerToList(elements, ()=>this.hideModal());

        this.doc.getElementById('modal-dialog').onclick = (e)=>{
            e.stopPropagation();
        };
    }
    
    toggleRadioButtons(isFade){
        //when radio button changed
        if(this.colorPickerDisabled != isFade){
            this.toggleColorPicker();
            this.colorPickerDisabled = !this.colorPickerDisabled;
            this.changeColorMode(isFade ? '1' : '0');
            if(!isFade){
                this.changeColor(this.defaultColor);
            }
        }
    }


    showProgressModal(title){
        const progressbar = '<div class=$$progress mr-3$$><div class=$$progress-bar progress-bar-blocks$$></div></div>';
        this.showModal(title, progressbar);
    }


    openWifiValueHelp(){
        this.showProgressModal('Scanning for WiFi networks...');
        this.sendHttpRequest('scanWifi', 'GET', null, (e)=>{
            try {
                this.hideModal();
                let response = JSON.parse(e.target.response);
                if(response.wifi.length > 0){
                    let htmlBody = '<div class=$$button-list$$>';
                    for (const wifi of response.wifi) {
                        htmlBody += '<button class=$$btn ml-2 btn-primary network-buttons$$ type=$$button$$ value=$$' + wifi.name + '$$>' + wifi.name + '</button>';
                    }
                    htmlBody +=    '</div>';
                    this.showModal('Available WiFi networks', htmlBody,'i');
                    this.addEventListenerToList(this.doc.getElementsByClassName('network-buttons'), (e)=>{
                        this.preselectSSID(e.target.value);
                        this.hideModal();
                    });
                }else{
                    this.showModal('Available WiFi networks', 'Can not find any WiFi networks','i');
                }
            } catch (error) {
                this.hideModal();
                this.showModal('An Error occurred!', 'Can not scan for WiFi networks.','e');
                console.error(error);
            }
        });
    }

    preselectSSID(wifiName){
        this.elements.ssid.value = wifiName;
    }
    
    submitWifi(){
        const ssid = this.elements.ssid.value;
        const wifipw = this.elements.wifipw.value;
        this.showProgressModal('Connecting...');
        
        if(this.isValidSSID(ssid) && this.isValidWifiPW(wifipw)){
            this.sendHttpRequest('/settingsPost', 'POST', ssid + ',.,' + wifipw,
                (e)=>{
                    this.hideModal();
                    
                    if(e.target.status != 200){
                        this.showModal('An Error occurred!', 'WiFi credentials are not correct.','e');
                    }else{
                        this.elements.networkForm.reset();

                        let prompt = 'Please connect your device to the same WiFi in order to continue.<br>';

                        try {
                            let response = JSON.parse(e.target.response);
                            prompt = 'Please connect your device to the WiFi: ' + response.wifi + ' in order to continue.<br>';
                            if(!!response.hostname && !!response.ip){
                                prompt += 'Use hostname: ' + response.hostname + ' or IP: ' + response.ip + ' to open up this site again in the new network.'
                            }
                        } catch (error) {
                            console.error(error);
                        }
                        this.showModal('WiFi connected!', prompt, 'i');
                        
                        this.doc.getElementById('settingsTabNavItem').style.display = 'none';
                        this.toggleTabs(this.doc.getElementById('home-tab'));
                    }
                },
                ()=>{
                    this.showModal('Error occurred!', 'WiFi credentials are not correct.','e');
                });
        }
    }
    
    setInitalConfig(){
        this.sendHttpRequest('getMode', 'GET', null, (e)=>{
            const config = {doFade: false, isAccessPoint: true};
            try {
                let response = JSON.parse(e.target.response);
                config.doFade = !!response.doFade;
                config.isAccessPoint = !response.wifiMode;
            } catch (error) {
                console.error(error);
            }
            if(config.doFade){
                this.toggleColorPicker();
                this.colorPickerDisabled = true;
                this.doc.getElementById('gridRadios1').checked = true;
            }else{
                this.doc.getElementById('gridRadios2').checked = true;
            }

            if(!config.isAccessPoint){
                this.doc.getElementById('settingsTabNavItem').style.display = 'none';
            }
        });
    }
    
    toggleTabs(selectedTab){
        if(selectedTab != this.activeTab){
            this.activeTab = selectedTab;
            const allTabs = this.doc.getElementsByClassName('nav-link');
            const allTabBodys = this.doc.getElementsByClassName('tab-pane');
            for (const tab of allTabs) {
                tab.classList.remove('active');
            }
            selectedTab.classList.add('active');
            for (const tabBody of allTabBodys) {
                if(tabBody.id == selectedTab.id.replace('-tab', '')){
                    tabBody.classList.add('active');
                    tabBody.classList.add('show');
                }else{
                    tabBody.classList.remove('active');
                    tabBody.classList.remove('show');
                }
            }
        }
    }
}


//Init Object
document.addEventListener('DOMContentLoaded', function(){
    const lampConfig = new LampConfig(document);
});