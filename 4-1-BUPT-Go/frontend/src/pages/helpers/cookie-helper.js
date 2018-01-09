
export default function () {
    var cookies = {};
    var rawCookies = document.cookie.split(';');
    for (var i = 0; i < rawCookies.length; i++) {
        var pair = rawCookies[i].trim().split('=');
        cookies[pair[0]] = pair[1];
    }
    return cookies;
}
