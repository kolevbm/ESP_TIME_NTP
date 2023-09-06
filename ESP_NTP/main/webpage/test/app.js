
let submitButton = document.getElementById('submitBtn');
// console.log(submitButton);

submitButton.addEventListener('click', submitAlarms);
/**
 *  Send the alarms entered into the time fields.
 */
function submitAlarms()
{
	// Get the alarms
	let alarm1 = $("#alarm1").val();
	let alarm2 = $("#alarm2").val();
	let alarm3 = $("#alarm3").val();
	let alarm4 = $("#alarm4").val();
	let alarm5 = $("#alarm5").val();
	let alarm6 = $("#alarm6").val();
	let alarm7 = $("#alarm7").val();
	
	 if(alarm1 === '' || alarm2 === '' || alarm3 === ''|| alarm4 === ''|| alarm5 === ''|| alarm6 === ''|| alarm7 === ''){
		 console.log(alarm1);
		alert("An alarm is left blank");
	 }
	
	
	$.ajax({
		url: '/submitAlarms.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		headers: {'alarm1': alarm1,
				'alarm2': alarm2,
				'alarm3': alarm3,
				'alarm4': alarm4,
				'alarm5': alarm5,
				'alarm6': alarm6,
				'alarm7': alarm7, },
		data: {'timestamp': Date.now()}
	});
}