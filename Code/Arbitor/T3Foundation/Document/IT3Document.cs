/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.Collections.Generic;
using System.Configuration;
using System.Diagnostics;
using System.IO;

namespace T3Foundation.Documents
{
    public enum ET3DocumentType
    {
        /// Basic text document. this can be used to hold public information about the application.
        ET3DT_TEXT = 0,
        ET3DT_XMLElement = 1,
        ET3DT_XML = 2,
        /// Interchangeable data format that we commonly use.
        ET3DT_DDL = 3,
        ET3DT_JSON = 4,
        /// Some sort of binary data format that the underlying application uses. this can range from: nspak, nslevel, nsasset, etc... 
        ET3DT_BINARY = 4,
    }
    /// <summary>
    /// Document Class for T3 Applications.
    /// This class defines HOW the data is being edited
    /// The Design is based off of a Workflow Driven Tool Design. (See remedy's Talk: https://www.youtube.com/watch?v=kAfb0yx07Po&t=926s)/// 
    /// </summary> 
    public interface IT3Document
    {
        public string DocumentName {get; set;}
        /// <summary>
        /// Function for registering UI Elements with data.
        /// Components that are registered with this function are REQUIRED to give us some sort of information to send and receive for editing.
        /// </summary>
        /// <param name="assetdata">type of data that the component should be managing. this will be sent over to the context for data retrieval. (e.g.: "asset::PhysicsDefinition" we are looking for the physics definition )</param>
        /// <typeparam name="T">Component To Register</typeparam>
        public void RegisterComponent<T>(string assetdata);
        public ET3DocumentType DocumentType {get; set; }
        /// <summary>
        /// Whether the document is currently being edited/checked out by another user.
        /// Mainly for applications that use Perforce like we do.
        /// </summary>
        /// <value></value>
        public bool isEditable {get; set; }
    }
}