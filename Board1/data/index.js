function updateData() {
	fetch('/status')
	.then(r => r.json())
	.then(d => {
		// Cập nhật Nhiệt độ & Độ ẩm
		document.getElementById('temp').innerText = d.t.toFixed(1);
		document.getElementById('hum').innerText = d.h.toFixed(1);

		// Cập nhật thiết bị 1 (Đèn)
		updateDevice(
			'card-dev1',
			'btn1',
			'status1',
			d.d1,
			'active-bulb',
			'Bật Thiết Bị',
			'Tắt Thiết Bị'
		);

		// Cập nhật thiết bị 2 (Quạt)
		updateDevice(
			'card-dev2',
			'btn2',
			'status2',
			d.d2,
			'active-fan',
			'Bật Thiết Bị',
			'Tắt Thiết Bị'
		);
	})
	.catch(err => {
		console.warn("Không thể lấy trạng thái từ ESP32:", err);
	});
}

function updateDevice(cardId, btnId, statusId, state, activeClass, textOff, textOn) {
	let card = document.getElementById(cardId);
	let btn = document.getElementById(btnId);
	let status = document.getElementById(statusId);

	if (state) {
		card.classList.add('active', activeClass);
		btn.querySelector('.btn-text').innerText = textOn;
		status.innerText = 'ON';
	} else {
		card.classList.remove('active', activeClass);
		btn.querySelector('.btn-text').innerText = textOff;
		status.innerText = 'OFF';
	}
}

function toggle(id) {
	fetch('/toggle' + id).then(updateData);
}

function resetWifi() {
	if(confirm("Xóa cấu hình WiFi và khởi động lại thiết bị về chế độ AP Setup?")) {
		fetch('/reset-wifi').then(() => {
			alert("Đang khởi động lại... Hãy kết nối vào WiFi AP của ESP32.");
		});
	}
}

// Bắt đầu cập nhật dữ liệu định kỳ mỗi 2 giây
setInterval(updateData, 2000);
window.onload = updateData;
