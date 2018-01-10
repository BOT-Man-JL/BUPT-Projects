<template>
  <div>
    <el-header style="text-align: center">
      <el-row type="flex" align="middle" justify="space-around">
        <el-col :span="4">
          <router-link :to="{ name:'userPage' }">
            <i class="el-icon-back"></i>
          </router-link>
        </el-col>
        <el-col :span="16">
          <h1>编辑文章</h1>
        </el-col>
        <el-col :span="4">
          <router-link :to="{ name:'searchPage' }">
            <i class="el-icon-search"></i>
          </router-link>
        </el-col>
      </el-row>
    </el-header>
    <div>
      <el-row class="input-row">
        <el-input size="small"
                  placeholder="文章的标题"
                  v-model="title">
        </el-input>
      </el-row>
      <el-row class="input-row">
        <el-select style="width: 49%" size="small"
                   v-model="category" placeholder="请选择类别">
          <el-option v-for="item in categoryOptions"
                     :key="item"
                     :label="item"
                     :value="item">
          </el-option>
        </el-select>
        <el-select style="width: 49%" size="small"
                   v-model="area" placeholder="请选择地区">
          <el-option v-for="item in areaOptions"
                     :key="item"
                     :label="item"
                     :value="item">
          </el-option>
        </el-select>
      </el-row>
      <el-row class="input-row">
        <el-input size="small" style="width: 69%"
                  placeholder="联系方式（可选）"
                  v-model="contact">
        </el-input>
        <el-input size="small" style="width: 29%"
                  placeholder="人均花费￥（可选）"
                  v-model="cost">
        </el-input>
      </el-row>
      <el-row class="input-row">
        <el-input size="small"
                  placeholder="详细地址（可选）"
                  v-model="location">
        </el-input>
      </el-row>
      <el-row class="input-row">
        <el-input type="textarea"
                  :autosize="{ minRows: 6, maxRows: 12 }"
                  placeholder="在此处输入内容"
                  v-model="text">
        </el-input>
      </el-row>
      <el-row class="input-row" v-if="previousImg">
        <img :src="previousImg" style="width: 80%" />
      </el-row>
      <el-row class="input-row">
        <el-upload action="/placeholder"
                   list-type="picture"
                   :multiple="false"
                   :on-change="onSelectImage"
                   :auto-upload="false">
          <el-tag v-if="id">
            更新图片
            <i class="el-icon-plus"></i>
          </el-tag>
          <el-tag v-else>
            上传图片
            <i class="el-icon-plus"></i>
          </el-tag>
        </el-upload>
      </el-row>
      <el-row class="input-row" type="flex" align="middle" justify="space-around" v-if="id">
        <el-button @click="onSubmit" style="width: 40%" type="primary">
          提交
          <i class="el-icon-success el-icon--right"></i>
        </el-button>
        <el-button @click="onDelete" style="width: 40%">
          删除
          <i class="el-icon-delete el-icon--right"></i>
        </el-button>
      </el-row>
      <el-row class="input-row" type="flex" align="middle" justify="space-around" v-else>
        <el-button @click="onSubmit" style="width: 80%" type="primary">
          提交
          <i class="el-icon-success el-icon--right"></i>
        </el-button>
      </el-row>
    </div>
  </div>
</template>

<script>
  import axios from 'axios'
  import ajaxPrompt from './helpers/ajax-helper'
  import options from '../../../common/article-common.json'

  export default {
    name: 'editPage',
    props: ['id'],
    data() {
      return {
        categoryOptions: options.categoryOptions,
        areaOptions: options.areaOptions,
        // article
        title: '',
        text: '',
        file: null,
        previousImg: null,
        // meta
        category: '',
        area: '',
        location: '',
        contact: '',
        cost: ''
      };
    },
    mounted() {
      document.title = '编辑文章 | BUPT Go';
      if (!this.id) {
        // Create Mode
        return;
      }

      // Edit Mode
      const url = '/article';
      const params = { id: this.id };

      ajaxPrompt(this, axios.get(url, { params }), (res) => {
        this.category = res.category;
        this.area = res.area;
        this.location = res.location;
        this.contact = res.contact;
        this.cost = res.cost;

        this.title = res.title;
        this.text = res.text;
        this.previousImg = res.img;
      });
    },
    methods: {
      postAction(url, data) {
        ajaxPrompt(this, axios.post(url, data), (res) => {
          this.$message({
            message: res.msg, showClose: true
          });
          this.$router.push({ name: 'userPage' });
        });
      },
      onSubmit() {
        const url = '/article/submit';
        const data = new FormData();
        if (this.id) {
          data.append('id', this.id);
        }
        data.append('category', this.category);
        data.append('area', this.area);
        data.append('location', this.location);
        data.append('contact', this.contact);
        data.append('cost', this.cost);

        data.append('title', this.title);
        data.append('text', this.text);
        data.append('image', this.file);

        this.postAction(url, data);
      },
      onDelete() {
        const url = '/article/delete';
        const data = { id: this.id };
        this.postAction(url, data);
      },
      onSelectImage(file, fileList) {
        if (fileList && fileList[0])
          this.file = fileList[0].raw;
        else
          this.file = null;
      }
    }
  }
</script>

<style scoped>
  .input-row {
    margin: 10px 0;
    text-align: center;
  }
</style>
