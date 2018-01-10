
export default function (vm, promise, callback) {
    const loading = vm.$loading({ lock: true });
    promise
        .then((res) => {
            callback(res.data);
            loading.close();
        }).catch((e) => {
            loading.close();
            vm.$message.error({
                message: e.response.data.err, showClose: true
            });
        });
}
