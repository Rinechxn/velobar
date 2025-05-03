// DateUtils.js
function getDayOfWeek(date) {
    const days = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    return days[date.getDay()];
}

function getMonthName(date) {
    const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
    return months[date.getMonth()];
}

function formatDate(date) {
    const day = date.getDate();
    const month = getMonthName(date);
    const dayOfWeek = getDayOfWeek(date);
    return `${dayOfWeek} ${month} ${day}`;
}