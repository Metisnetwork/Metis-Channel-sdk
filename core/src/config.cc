#include "config.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
using namespace rapidjson;

#include <algorithm>
#include <unistd.h>

bool is_file_exist(const string& filepath) {
  if (filepath.empty())
    return false;
  return (access(filepath.c_str(), F_OK) == 0);
}
void if_key_not_exist_then_exit(bool must_exist, const char* key) {
  if (must_exist) {
    throw "key[" + string(key) + "] not exist!";
  }
}
std::string GetString(
  rapidjson::Value& v,
  const char* key,
  const char* default_value = "",
  bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetString();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return std::string(default_value);
}
int GetInt(rapidjson::Value& v, const char* key, int default_value = 0, bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetInt();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return default_value;
}
float GetFloat(
  rapidjson::Value& v,
  const char* key,
  float default_value = 0.0f,
  bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetFloat();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return default_value;
}
bool GetBool(
  rapidjson::Value& v,
  const char* key,
  bool default_value = 0,
  bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetBool();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return default_value;
}

ComputeNodeConfig::ComputeNodeConfig() {}

std::string ComputeNodeConfig::to_string() {
  std::stringstream sss;

  for (int i = 0; i < P.size(); i++) {
    sss << "\n        P" << i << " NAME: " << P[i].NAME;
    sss << "\n        P" << i << " ADDRESS: " << P[i].ADDRESS;
  }
  sss << "\n";
  return sss.str();
}

int ComputeNodeConfig::GetNodeIndex(const string& node_id) {
  for (int i = 0; i < P.size(); i++) {
    if (node_id == P[i].NODE_ID)
      return i;
  }
  return -1;
}

std::string ResultNodeConfig::to_string() {
  std::stringstream sss;

  for (int i = 0; i < P.size(); i++) {
    sss << "\n        P" << i << " NAME: " << P[i].NAME;
    sss << "\n        P" << i << " ADDRESS: " << P[i].ADDRESS;
  }
  sss << "\n";
  return sss.str();
}

std::string DataNodeConfig::to_string() {
  std::stringstream sss;

  for (int i = 0; i < P.size(); i++) {
    sss << "\n        P" << i << " NAME: " << P[i].NAME;
    sss << "\n        P" << i << " ADDRESS: " << P[i].ADDRESS;
  }
  sss << "\n";
  return sss.str();
}

ChannelConfig::ChannelConfig(const string& node_id, const string& config_json) {
  //! @attention use node_id__ID = node_id
  bool ret = load(node_id, config_json);
  if (!ret) {
    HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_PARSE_FILE, "");
  }
  node_id_ = node_id;
}
ChannelConfig::ChannelConfig(const string& config_json) {
  //! @attention use node_id__ID in config_json
  const string node_id = "";
  bool ret = load(node_id, config_json);
  if (!ret) 
  {
    HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_PARSE_FILE, "");
  }
}

vector<NODE_TYPE> ChannelConfig::GetNodeType(const string& node_id) {
  vector<NODE_TYPE> node_types;
  for (int i = 0; i < data_config_.P.size(); i++) {
    if (node_id == data_config_.P[i].NODE_ID)
      node_types.emplace_back(NODE_TYPE_DATA);
  }

  for (int i = 0; i < compute_config_.P.size(); i++) {
    if (node_id == compute_config_.P[i].NODE_ID)
      node_types.emplace_back(NODE_TYPE_COMPUTE);
  }

  for (int i = 0; i < result_config_.P.size(); i++) {
    if (node_id == result_config_.P[i].NODE_ID)
      node_types.emplace_back(NODE_TYPE_RESULT);
  }

  return node_types;
}

const Node& ChannelConfig::GetNode(const string& node_id) {
  for (int i = 0; i < data_config_.P.size(); i++) {
    if (node_id == data_config_.P[i].NODE_ID)
      return data_config_.P[i];
  //  cout << "data node id:" << data_config_.P[i].NODE_ID << endl;
  }

  for (int i = 0; i < compute_config_.P.size(); i++) {
    if (node_id == compute_config_.P[i].NODE_ID)
      return compute_config_.P[i];
  //  cout << "compute node id:" << compute_config_.P[i].NODE_ID << endl;
  }

  for (int i = 0; i < result_config_.P.size(); i++) {
    if (node_id == result_config_.P[i].NODE_ID)
      return result_config_.P[i];
  //  cout << "result node id:" << result_config_.P[i].NODE_ID << endl;
  }
  // cout << "node_id: " << node_id << endl;
  HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_NO_FIND_NID, task_id_, node_id.c_str());
}

bool ChannelConfig::load(const string& node_id, const string& config_file) {
  node_id_ = node_id;
  // config_json: json-file or json-string

  string sjson(config_file);
  ifstream ifile(config_file);
  if (!ifile.is_open()) {
    //log_warn << "open " << config_file << " error!\n";
    // cout << "try to load as json string" << endl;
  } else {
    sjson = "";
    while (!ifile.eof()) {
      string s;
      getline(ifile, s);
      sjson += s;
    }
    ifile.close();
  }

  Document doc;
  if (doc.Parse(sjson.data()).HasParseError()) {
    cout << "parser " << config_file << " error!\n";
    return false;
  }

  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  doc.Accept(writer);
  string data = buffer.GetString();
  // cout << "Config Source String:\n" << data << endl;

  if (!parse(doc)) {
    cout << "parse error" << endl;
    return false;
  }

  return true;
}

bool ChannelConfig::parse_node_info(Document& doc) 
{
  if (doc.HasMember("NODE_INFO") && doc["NODE_INFO"].IsArray()) 
  {
    Value& Nodes = doc["NODE_INFO"];
    string ca_cert = GetString(doc, "ROOT_CERT", root_cert_.c_str(), false);
    // nodes
    for (int i = 0; i < Nodes.Size(); i++) 
    {
      NodeInfoConfig cfg;
      Value& Node = Nodes[i];
      cfg.node_.NODE_ID = GetString(Node, "NODE_ID", "", false);
      // cout << "node info parse:" << cfg.node_.NODE_ID << endl;
      cfg.node_.ADDRESS = GetString(Node, "ADDRESS", "", false);
      cfg.node_.PUBLIC_IP = GetString(Node, "PUBLIC_IP", "127.0.0.1", false);
      cfg.node_.VIA = GetString(Node, "VIA", "", false);
      cfg.node_.GRICER2 = GetString(Node, "GRICER2", "", false);
      cfg.node_.ICEGRID = GetString(Node, "ICEGRID", "", false);
      cfg.node_.CERT_DIR = GetString(Node, "CERT_DIR", "", false);
      cfg.node_.CA_CERT_PATH = ca_cert;

      cfg.node_.SERVER_KEY_PATH = GetString(Node, "SERVER_KEY", "", false);
      cfg.node_.SERVER_CERT_PATH = GetString(Node, "SERVER_CERT", "", false);
      cfg.node_.CLIENT_KEY_PATH = GetString(Node, "CLIENT_KEY", "", false);
      cfg.node_.CLIENT_CERT_PATH = GetString(Node, "CLIENT_CERT", "", false);
      cfg.node_.PASSWORD = GetString(Node, "PASSWORD", "", false);
    
      #if(2 == SSL_TYPE)  
      {
        cfg.node_.SERVER_SIGN_KEY_PATH = GetString(Node, "SERVER_SIGN_KEY", "", false);
        cfg.node_.SERVER_SIGN_CERT_PATH = GetString(Node, "SERVER_SIGN_CERT", "", false);
        cfg.node_.SERVER_ENC_KEY_PATH = GetString(Node, "SERVER_ENC_KEY", "", false);
        cfg.node_.SERVER_ENC_CERT_PATH = GetString(Node, "SERVER_ENC_CERT", "", false);
        cfg.node_.CLIENT_SIGN_KEY_PATH = GetString(Node, "CLIENT_SIGN_KEY", "", false);
        cfg.node_.CLIENT_SIGN_CERT_PATH = GetString(Node, "CLIENT_SIGN_CERT", "", false);
        cfg.node_.CLIENT_ENC_KEY_PATH = GetString(Node, "CLIENT_ENC_KEY", "", false);
        cfg.node_.CLIENT_ENC_CERT_PATH = GetString(Node, "CLIENT_ENC_CERT", "", false);
      }
      #endif

      node_info_config_.insert(std::pair<string, NodeInfoConfig>(cfg.node_.NODE_ID, cfg));
      nodeid_to_via_.insert(std::pair<string, string>(cfg.node_.NODE_ID, cfg.node_.VIA));
      nodeid_to_glacier2_.insert(std::pair<string, string>(cfg.node_.NODE_ID, cfg.node_.GRICER2));
      nodeid_to_icegrid_.insert(std::pair<string, string>(cfg.node_.NODE_ID, cfg.node_.ICEGRID));
    }
    // cout << "parse " << Nodes.Size() << " node info success" << endl;

    if (doc.HasMember("VIA_INFO") && doc["VIA_INFO"].IsObject()) 
    {
      Value& Vias = doc["VIA_INFO"];

      for (auto iter = Vias.MemberBegin(); iter != Vias.MemberEnd(); iter++) 
      {
        string name = iter->name.GetString();
        string value = iter->value.GetString();
        via_to_address_.insert(std::pair<string, string>(name, value));
      }
    }
    
    if (doc.HasMember("GRICER2_INFO") && doc["GRICER2_INFO"].IsObject()) 
    {
      Value& list_glacier2 = doc["GRICER2_INFO"];
      for (auto iter = list_glacier2.MemberBegin(); iter != list_glacier2.MemberEnd(); iter++) 
      {
        string glacier2_name = iter->name.GetString();
        if(list_glacier2[glacier2_name.c_str()].IsObject()) {
          Value& glacier2_infos = list_glacier2[glacier2_name.c_str()];
          IcePlugCfg glacier2_cfg;
          for (auto iter = glacier2_infos.MemberBegin(); 
              iter != glacier2_infos.MemberEnd(); iter++) 
          {
            string name = iter->name.GetString();
            string value = iter->value.GetString();
            if("APPNAME" == name) {
              glacier2_cfg.AppName_ = value;
            } else if ("IP" == name) {
              glacier2_cfg.Ip_ = value;
            } else if ("PORT" == name) {
              glacier2_cfg.Port_ = value;
            }
            // cout << "name:" << name << ", value:" << value << endl;
          }
          glacier2_to_info_.insert(std::pair<string, IcePlugCfg>(glacier2_name, glacier2_cfg));
        }
      }
    }

    if (doc.HasMember("ICE_GRID_INFO") && doc["ICE_GRID_INFO"].IsObject()) 
    {
      Value& list_iceGrid = doc["ICE_GRID_INFO"];
      for (auto iter = list_iceGrid.MemberBegin(); iter != list_iceGrid.MemberEnd(); iter++) 
      {
        string grid_name = iter->name.GetString();
        if(list_iceGrid[grid_name.c_str()].IsObject()) {
          Value& grid_infos = list_iceGrid[grid_name.c_str()];
          IcePlugCfg grid_cfg;
          for (auto iter = grid_infos.MemberBegin(); 
              iter != grid_infos.MemberEnd(); iter++) 
          {
            string name = iter->name.GetString();
            string value = iter->value.GetString();
            if("APPNAME" == name) {
              grid_cfg.AppName_ = value;
            } else if ("IP" == name) {
              grid_cfg.Ip_ = value;
            } else if ("PORT" == name) {
              grid_cfg.Port_ = value;
            }
            // cout << "name:" << name << ", value:" << value << endl;
          }
          icegrid_to_info_.insert(std::pair<string, IcePlugCfg>(grid_name, grid_cfg));
        }
      }
    }
  }
  return true;
}

bool ChannelConfig::parse_data(Document& doc) {
  if (doc.HasMember("DATA_NODES") && doc["DATA_NODES"].IsArray()) {
    Value& Nodes = doc["DATA_NODES"];

    // nodes
    data_nodes_.resize(Nodes.Size());
    data_config_.P.resize(Nodes.Size());
    for (int i = 0; i < Nodes.Size(); i++) {
      Value& Node = Nodes[i];
      data_nodes_[i] = Node.GetString();
      if (node_info_config_.find(data_nodes_[i]) != node_info_config_.end()) {
        data_config_.P[i].copy_from(node_info_config_[data_nodes_[i]].node_);
      } else {
        // cout << "can not find node info, node id:" << data_nodes_[i] << endl;
        HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_NO_FIND_NID, task_id_, data_nodes_[i].c_str());
      }
      // Save the id of the node that participates in the task
      if(task_nodes_.find(data_nodes_[i]) == task_nodes_.end()) {
        task_nodes_.insert(data_nodes_[i]);
      }
    }
    // cout << "parse " << Nodes.Size() << " data success" << endl;

  }
  return true;
}

bool ChannelConfig::parse_compute(Document& doc) {
  if (doc.HasMember("COMPUTATION_NODES") && doc["COMPUTATION_NODES"].IsObject()) {
    Value& Nodes = doc["COMPUTATION_NODES"];

    // nodes
    for (auto iter = Nodes.MemberBegin(); iter != Nodes.MemberEnd(); iter++) {
      string name = iter->name.GetString();
      string value = iter->value.GetString();
      int party = -1;
      if (value == "P0") {
        party = 0;
      } else if (value == "P1") {
        party = 1;
      } else if (value == "P2") {
        party = 2;
      }
      compute_nodes_.insert(std::pair<string, int>(name, party));
    }

    compute_config_.P.resize(compute_nodes_.size());
    int i = 0;
    for (auto iter = compute_nodes_.begin(); iter != compute_nodes_.end(); iter++, i++) {
      if (node_info_config_.find(iter->first) != node_info_config_.end()) {
        compute_config_.P[i].copy_from(node_info_config_[iter->first].node_);
      } else {
        // cout << "can not find node info, node id:" << iter->first << endl;
        HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_NO_FIND_NID, task_id_, iter->first.c_str());
      }
      // Save the id of the node that participates in the task
      if(task_nodes_.find(iter->first) == task_nodes_.end()) {
        task_nodes_.insert(iter->first);
      }
    }
    // cout << "parse " << " computation success" << endl;
  }
  return true;
}

bool ChannelConfig::parse_result(Document& doc) {
  if (doc.HasMember("RESULT_NODES") && doc["RESULT_NODES"].IsArray()) {
    Value& Nodes = doc["RESULT_NODES"];

    // nodes
    result_nodes_.resize(Nodes.Size());
    result_config_.P.resize(Nodes.Size());
    for (int i = 0; i < Nodes.Size(); i++) {
      Value& Node = Nodes[i];
      result_nodes_[i] = Node.GetString();
      if (node_info_config_.find(result_nodes_[i]) != node_info_config_.end()) {
        result_config_.P[i].copy_from(node_info_config_[result_nodes_[i]].node_);
      } else {
        // cout << "can not find node info, node id:" << result_nodes_[i] << endl;
        HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_NO_FIND_NID, task_id_, result_nodes_[i].c_str());
      }

      // Save the id of the node that participates in the task
      if(task_nodes_.find(result_nodes_[i]) == task_nodes_.end()) {
        task_nodes_.insert(result_nodes_[i]);
      }
    }
    // cout << "parse " << Nodes.Size() << " result success" << endl;
  }
  return true;
}

bool ChannelConfig::parse_policy(Document& doc) {
  if (doc.HasMember("POLICY")) {
    if(doc["POLICY"].IsString()) {
      policy_type_ = GetString(doc, "POLICY", "all", false);
    } else if(doc["POLICY"].IsObject()) {
      Value& dict_policy = doc["POLICY"];

      for (auto iter = dict_policy.MemberBegin(); iter != dict_policy.MemberEnd(); iter++) 
      {
        string nodeid = iter->name.GetString();
        set<string> nodeids;
        if(dict_policy[nodeid.c_str()].IsArray()) {

          Value& ServerNodes = dict_policy[nodeid.c_str()];
          // nodes
          for (int i = 0; i < ServerNodes.Size(); i++) 
          {
            string strTmp;
            Value& Node = ServerNodes[i];
            strTmp = Node.GetString();
            nodeids.emplace(strTmp);
          }
        }
        map_policy_.emplace(nodeid, nodeids);
      }
    }
  }
  return true;
}

bool ChannelConfig::parse(Document& doc) {
  //! @todo the node_id__ID field in CONFIG.json have not yet used
  task_id_ = GetString(doc, "TASK_ID", "", false);
  root_cert_ = GetString(doc, "ROOT_CERT", "", false);
  log_level_ = GetInt(doc, "LOG_LEVEL", 2, false);
  buffer_size_ = GetInt(doc, "BUFFER_SIZE", 8 * 1024, false);
  ping_time_ = GetFloat(doc, "PING_TIME", 1.0, false);
  send_timeout_ = GetFloat(doc, "SEND_TIMEOUT", 5.0, false);
  conn_timeout_ = GetFloat(doc, "CONNECT_TIMEOUT", 5.0, false);
  conn_sync_ = GetBool(doc, "CONNECT_SYNC", false, false);
  
  if(2 < log_level_)
    log_level_ = 2;
  if (!parse_node_info(doc)) {
    cout << "parse node info error" << endl;
  }

  if (!parse_data(doc)) {
    cout << "parse data error" << endl;
    return false;
  }

  if (!parse_compute(doc)) {
    cout << "parse compute error" << endl;
    return false;
  }

  if (!parse_result(doc)) {
    cout << "parse result error" << endl;
    return false;
  }

  if (!parse_policy(doc)) {
    cout << "parse connect policy error" << endl;
    return false;
  }

  // fmt_print();
  return true;
}

void ChannelConfig::fmt_print() {
  cout << "=======================================" << endl;
  cout << "          node_id_: " << node_id_ << endl;
  cout << data_config_.to_string();
  cout << compute_config_.to_string();
  cout << result_config_.to_string();
  cout << "=======================================" << endl;
}

void ChannelConfig::CopyNodeInfo(NodeInfo& node_info, const Node& nodeInfo) 
{
  node_info.id = nodeInfo.NODE_ID;
  node_info.address = nodeInfo.ADDRESS;
  node_info.public_ip_ = nodeInfo.PUBLIC_IP;
  node_info.ca_cert_path_ = root_cert_;
  node_info.cert_dir_ = nodeInfo.CERT_DIR;
  node_info.server_key_path_ = nodeInfo.SERVER_KEY_PATH;
  node_info.server_cert_path_ = nodeInfo.SERVER_CERT_PATH;
  node_info.client_key_path_ = nodeInfo.CLIENT_KEY_PATH;
  node_info.client_cert_path_ = nodeInfo.CLIENT_CERT_PATH;
  node_info.password_ = nodeInfo.PASSWORD;

  #if(2 == SSL_TYPE)
  {
    node_info.server_sign_key_path_ = nodeInfo.SERVER_SIGN_KEY_PATH;
    node_info.server_sign_cert_path_ = nodeInfo.SERVER_SIGN_CERT_PATH;
    node_info.server_enc_key_path_ = nodeInfo.SERVER_ENC_KEY_PATH;
    node_info.server_enc_cert_path_ = nodeInfo.SERVER_ENC_CERT_PATH;
    node_info.client_sign_key_path_ = nodeInfo.CLIENT_SIGN_KEY_PATH;
    node_info.client_sign_cert_path_ = nodeInfo.CLIENT_SIGN_CERT_PATH;
    node_info.client_enc_key_path_ = nodeInfo.CLIENT_ENC_KEY_PATH;
    node_info.client_enc_cert_path_ = nodeInfo.CLIENT_ENC_CERT_PATH;
  }
  #endif

  // 获取本节点对应的via地址
  string via_name = nodeid_to_via_[node_info.id];
  node_info.via_address = via_to_address_[via_name];
  // 获取本节点对应的glacier2地址
  string glacier2_name = nodeid_to_glacier2_[node_info.id];
  node_info.glacier2_info = glacier2_to_info_[glacier2_name];

  // 获取本节点对应的IceGrid地址
  string ice_grid_name = nodeid_to_icegrid_[node_info.id];
  node_info.ice_grid_info = icegrid_to_info_[ice_grid_name];
}

bool ChannelConfig::isNodeType(const vector<NODE_TYPE>& vec_node_types, const NODE_TYPE nodeType)
{
  for(int i=0; i < vec_node_types.size(); ++i)
  {
    if(vec_node_types[i] == nodeType)
    {
      return true;
    }
  }
  return false;
}

void GetCertInfosFromNode(ViaInfo& viaTmp, const Node& node)
{
  viaTmp.cert_dir_ = node.CERT_DIR;
  viaTmp.ca_cert_path_ = node.CA_CERT_PATH;
  viaTmp.server_cert_path_ = node.SERVER_CERT_PATH;
  viaTmp.client_key_path_ = node.CLIENT_KEY_PATH;
  viaTmp.client_cert_path_ = node.CLIENT_CERT_PATH;
  viaTmp.password_ = node.PASSWORD;
  #if(2 == SSL_TYPE)
  {
    viaTmp.server_cert_path_ = node.CA_CERT_PATH;
    viaTmp.client_sign_key_path_ = node.CLIENT_SIGN_KEY_PATH;
    viaTmp.client_sign_cert_path_ = node.CLIENT_SIGN_CERT_PATH;
    viaTmp.client_enc_key_path_ = node.CLIENT_ENC_KEY_PATH;
    viaTmp.client_enc_cert_path_ = node.CLIENT_ENC_CERT_PATH;
  }
  #endif
}

bool ChannelConfig::GetNodeInfos(set<string>& clientNodeIds, set<ViaInfo>& serverInfos, 
    const string& node_id)
{
  if(0 == map_policy_.size()) {
    GetAllNodeInfos(clientNodeIds, serverInfos, node_id);
  } else {
    for(auto& iter_map: map_policy_) {
      set<string> clientTmp;
      bool isAlreadyGet = false;
      // find self nodeid
      if(false == isAlreadyGet && (iter_map.first == node_id) ) {
        // get service nodeid
        for(auto& iter_nid: iter_map.second) {
          GetInfoByNodeId(clientTmp, serverInfos, iter_nid);
        }
        isAlreadyGet = true;
      }
      // find the client nodeid connected to self node.
      if( (iter_map.first != node_id) && (iter_map.second.find(node_id) != iter_map.second.end()) ) {
        clientNodeIds.emplace(iter_map.first);
      }
    }
    // The node must be a client or a server
    if(clientNodeIds.empty() && serverInfos.empty()) {
      HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_NEITHER_CLIENT_SERVER, "", node_id.c_str());
    }
  }
  
  return true;
}

bool ChannelConfig::GetInfoByNodeId(set<string>& clientNodeIds, set<ViaInfo>& serverInfos, 
    const string& node_id)
{
  if(node_info_config_.find(node_id) == node_info_config_.end()) {
     HANDLE_EXCEPTION_EVENT(C_EVENT_CODE_NO_FIND_NID, task_id_, node_id.c_str());
  }

  string via = nodeid_to_via_[node_id];
  string glacier2 = nodeid_to_glacier2_[node_id];
  ViaInfo viaTmp;
  viaTmp.id = node_id;
  viaTmp.via = via;
  viaTmp.glacier2_info = glacier2_to_info_[glacier2];

  const Node& node = node_info_config_[node_id].node_;
  viaTmp.via_address = node.ADDRESS;
  GetCertInfosFromNode(viaTmp, node);
  serverInfos.emplace(viaTmp);
  clientNodeIds.emplace(node_id);
  return true;
}

bool ChannelConfig::GetAllNodeInfos(set<string>& clientNodeIds, set<ViaInfo>& serverInfos, 
    const string& node_id)
{
  // set<string> nodeid_set;
  // // 遍历计算节点
  // for (int i = 0; i < data_config_.P.size(); i++) 
  // {
  //   // 获取计算节点的nodeid
  //   string nid = data_config_.P[i].NODE_ID;
  //   string via = nodeid_to_via_[nid];
  //   string glacier2 = nodeid_to_glacier2_[nid];
  //   if (node_id != nid && nodeid_set.find(nid) == nodeid_set.end()) 
  //   {
  //     ViaInfo viaTmp;
  //     viaTmp.id = nid;
  //     viaTmp.via = via;
  //     viaTmp.via_address = via_to_address_[via];
  //     if(viaTmp.via_address.empty()) {
  //       viaTmp.via_address = data_config_.P[i].ADDRESS;
  //     }
  //     viaTmp.glacier2_info = glacier2_to_info_[glacier2];

  //     const Node& node = node_info_config_[nid].node_;
  //     GetCertInfosFromNode(viaTmp, node);
  //     // cout << "id: " << nid << ", via: " << viaTmp.via << ", via_address: " << viaTmp.via_address << endl;
  //     serverInfos.push_back(viaTmp);
  //     clientNodeIds.push_back(nid);
  //     nodeid_set.insert(nid);
  //   }
  // }

  // // 遍历计算节点
  // for (int i = 0; i < compute_config_.P.size(); i++) 
  // {
  //   // 获取计算节点的nodeid
  //   string nid = compute_config_.P[i].NODE_ID;
  //   string via = nodeid_to_via_[nid];
  //   string glacier2 = nodeid_to_glacier2_[nid];
  //   if (node_id != nid && nodeid_set.find(nid) == nodeid_set.end()) 
  //   {
  //     ViaInfo viaTmp;
  //     viaTmp.id = nid;
  //     viaTmp.via = via;
  //     viaTmp.via_address = via_to_address_[via];
  //     if(viaTmp.via_address.empty()) {
  //       viaTmp.via_address = compute_config_.P[i].ADDRESS;
  //     }
  //     viaTmp.glacier2_info = glacier2_to_info_[glacier2];
  //     const Node& node = node_info_config_[nid].node_;
  //     GetCertInfosFromNode(viaTmp, node);
  //     // cout << "id: " << nid << ", via: " << viaTmp.via << ", via_address: " << viaTmp.via_address << endl;
  //     serverInfos.push_back(viaTmp);
  //     clientNodeIds.push_back(nid);
  //     nodeid_set.insert(nid);
  //   }
  // }
  
  // for (int i = 0; i < result_config_.P.size(); i++) 
  // {
  //   // cout << "handle compute node" << endl;
  //   string nid = result_config_.P[i].NODE_ID;
  //   string via = nodeid_to_via_[nid];
  //   string glacier2 = nodeid_to_glacier2_[nid];
  //   if (node_id != nid && nodeid_set.find(nid) == nodeid_set.end()) 
  //   {
  //     ViaInfo viaTmp;
  //     viaTmp.id = nid;
  //     // 节点所在via
  //     viaTmp.via = via;
  //     // via信息
  //     viaTmp.via_address = via_to_address_[viaTmp.via];
  //     if(viaTmp.via_address.empty()) {
  //       viaTmp.via_address = result_config_.P[i].ADDRESS;
  //     }
  //     viaTmp.glacier2_info = glacier2_to_info_[glacier2];
  //     const Node& node = node_info_config_[nid].node_;
  //     GetCertInfosFromNode(viaTmp, node);
  //     // cout << "id: " << nid << ", via: " << viaTmp.via << ", address: " << viaTmp.via_address << endl;
  //     serverInfos.push_back(viaTmp);
  //     clientNodeIds.emplace_back(nid);
  //     nodeid_set.emplace(nid);
  //   }
  // }  

  for(auto& nid: task_nodes_) 
  {
    if (node_id != nid) 
    {
      GetInfoByNodeId(clientNodeIds, serverInfos, nid);
    }
  }
  return true;
}
