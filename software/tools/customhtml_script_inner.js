// This is the JavaScript source for the web interface.
//
// It should be automated at some point but until then, if you make
// changes here, you'll need to reflect them in Iot.cpp. To do so,
// minify this through eg https://closure-compiler.appspot.com/home
// and paste the result for CUSTOMHTML_SCRIPT_INNER in Iot.cpp
// (make sure to escape backslashes and double quotes!).

document.addEventListener(
    'DOMContentLoaded',
    function (e) {

        // Replace text inputs with desired type
        document.querySelectorAll("[data-type]").forEach((el) => {
          el.type = el.getAttribute("data-type");
        });

        // Add a toggle to reveal values in password fields.
        document.querySelectorAll('input[type=password]').forEach((p) => {
            const btn = document.createElement('input');
            btn.classList.add('pwtoggle');
            btn.type = 'button';
            btn.value = '👁️';
            p.insertAdjacentElement('afterend', btn);
            btn.onclick = function() { if (p.type === 'password') { p.type = 'text'; btn.value = '🔒'; } else { p.type = 'password'; btn.value = '👁️'; } }
        });

        // Disables the submit button and update its label when saving.
        const formEl = document.querySelector("form");
        if (formEl) {
            formEl.addEventListener('submit', () => {
                let btn = document.querySelector("button[type=submit]");
                btn.innerText = "Saving..."; 
                btn.toggleAttribute("disabled", true);
            });   
        }

        // Replaces inputs with data-options attribute with proper select elements.
        // Inspiration: https://github.com/prampec/IotWebConf/issues/29#issuecomment-533751475

        const customSelects = document.querySelectorAll('input[data-options]');

        customSelects.forEach((customSelect) => {
            const defaultValue = customSelect.value;
            const options = customSelect.getAttribute('data-options').split('|');
            const selectEl = document.createElement('select');
            selectEl.name = customSelect.name;
            selectEl.id = customSelect.id;
            if (defaultValue === '') {
                selectEl.appendChild(document.createElement('option'))
            }
            let optgroupEl = null;
            options.forEach((option, index) => {
                const multipart = option.split('/');
                let optionLabel = option;
                if (multipart.length > 1) {
                    const groupLabel = multipart.splice(0,1)[0];
                    // Start new optgroup ?
                    if (!optgroupEl || (groupLabel != optgroupEl.label)) {
                        optgroupEl && selectEl.appendChild(optgroupEl);
                        optgroupEl = document.createElement('optgroup');
                        optgroupEl.label = groupLabel;
                    }
                    optionLabel = multipart.join(" / ");
                } else {
                    // No multipart label, close current group.
                    if (optgroupEl) {
                        selectEl.appendChild(optgroupEl);
                        optgroupEl = null;
                    }
                }
                const optionEl = document.createElement('option');
                optionEl.value = index;
                optionEl.innerText = optionLabel;
                if (index == defaultValue) {
                    optionEl.toggleAttribute('selected');
                }
                if (optgroupEl) {
                    optgroupEl.appendChild(optionEl);
                } else {
                    selectEl.appendChild(optionEl);
                }
            });
            if (optgroupEl) {
                selectEl.appendChild(optgroupEl);
            }
            customSelect.id += '-d';
            customSelect.insertAdjacentElement('beforebegin', selectEl);
            customSelect.parentElement.removeChild(customSelect);
        });

        // Adds a label next to Range inputs
        document.querySelectorAll("input[type=range]").forEach((inputEl) => {
            const labels = inputEl.getAttribute("data-labels");
            const labelList = labels && labels.split("|");
            const setLabel = () => {
                inputEl.setAttribute("data-label", labelList ? labelList[parseInt(inputEl.value, 10)] || inputEl.value : inputEl.value);
            };
            inputEl.oninput = setLabel;
            setLabel();
        });

        // // Show/hide config fields based on NTP setting
        const ntpEl = document.getElementById("ntp_enabled");
        if (ntpEl) {
            const toggleTimeFields = (ntpEnabled) => {
                document.getElementById('time').parentElement.style.display = ntpEnabled ? 'none' : '';
                document.getElementById('timezone').parentElement.style.display = ntpEnabled ? '' : 'none';
            };
            ntpEl.addEventListener('change', (e) => {
                toggleTimeFields(ntpEl.value == 1);
            });
            toggleTimeFields(ntpEl.value == 1);
        }

        // Apply the selected color to the clock logo :)
        const colorInputEl = document.querySelector('input[type=color]');
        if (colorInputEl) {
            const updateLogoColor = () => {
                document.querySelector('.logoContainer').style.backgroundColor = colorInputEl.value
            };
            colorInputEl.addEventListener('input', updateLogoColor);
            updateLogoColor();
        }
    }
)