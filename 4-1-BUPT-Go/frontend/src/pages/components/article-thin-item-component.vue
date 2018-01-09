<template>
  <div>
  <el-row type="flex" align="bottom" justify="space-around">
    <el-col :span="16" align="left">
      <router-link :to="{ name:'articlePage', query: { id: id },
                      params: { title: title, img: img, author: author } }">
        <h2>{{ title }}</h2>
        <p><small>{{ timestamp }}</small></p>
      </router-link>
    </el-col>
    <el-col :span="8" align="right">
      <router-link :to="{ name:'editPage', query: { id: id } }">
        <el-button icon="el-icon-edit" size="mini" />
      </router-link>
      <el-button @click="onDelete" icon="el-icon-delete" size="mini" />
    </el-col>
  </el-row>
  <hr height="1px" />
  </div>
</template>

<script>
  import axios from 'axios'
  export default {
    name: 'articleThinItemComponent',
    props: ['id', 'title', 'timestamp'],
    methods: {
      onDelete() {
        const url = '/article/delete';
        const data = { id: this.id };

        const loading = this.$loading({ lock: true });
        axios.post(url, data).then((res) => {
          this.$message({
            message: res.data.msg, showClose: true
          });
          this.$emit('remove-item');

          loading.close();
        }).catch((e) => {
          loading.close();
          this.$message.error({
            message: e.response.data.err, showClose: true
          });
        });
      }
    }
  }
</script>

<style scoped>
  a {
    color: #808080;
    text-decoration: none;
  }
</style>
