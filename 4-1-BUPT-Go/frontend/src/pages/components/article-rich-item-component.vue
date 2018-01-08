<template>
  <router-link :to="{ name:'articlePage', query: { id: id },
                      params: { title: title, img: img, author: author } }">
    <div class="list-item">
      <el-row type="flex" align="middle" justify="space-around">
        <img v-bind:src="img" style="width: 100%">
      </el-row>
      <el-row type="flex" align="middle" justify="space-around">
        <el-col :span="24">
          <h2>{{ title }}</h2>
        </el-col>
      </el-row>
      <el-row type="flex" align="middle" justify="space-around">
        <el-col :span="12" align="left">
          <h4>作者：{{ author }}</h4>
        </el-col>
        <el-col :span="12" align="right">
          <el-tag>{{ category }}</el-tag>
          <el-tag>{{ area }}</el-tag>
        </el-col>
      </el-row>
    </div>
  </router-link>
</template>

<script>
  export default {
    name: 'articleRichItemComponent',
    props: ['id', 'title', 'img', 'author', 'category', 'area']
  }
</script>

<style scoped>
  .list-item {
    box-shadow: 0px 2px 5px 0px rgba(0,0,0,0.16), 0px 2px 10px 0px rgba(0,0,0,0.12);
    padding: 20px 30px;
    margin: 20px 0;
  }

  a {
    color: #808080;
    text-decoration: none;
  }
</style>
